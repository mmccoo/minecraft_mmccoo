

#include <BiomeImage.h>
#include <biome.h>

#include <Magick++.h>
// I really don't like using namespace... I prefer to fully qualify.
// but it won't compile without.
using namespace Magick;

#include <sstream>
#include <fstream>
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

BiomeImage::BiomeImage()
{
    Magick::InitializeMagick("./foo");
}

void BiomeImage::add_chunk(int chunkx, int chunkz, Grid16 biomes)
{
    chunks[chunkx][chunkz] = biomes;

    Magick::Image image(Magick::Geometry(16*pixels_per_block, 16*pixels_per_block), Magick::Color(0,0,0,0) );

    image.strokeWidth(0);

    for(int x=0; x<16; x++) {
        for(int y=0; y<16; y++) {
            auto b = get_biome(biomes[x][y]);
            Magick::Color c(QuantumRange/256*b.color.r,
                            QuantumRange/256*b.color.g,
                            QuantumRange/256*b.color.b,
                            QuantumRange);
            image.fillColor(c); // Fill color
            image.draw( Magick::DrawableRectangle(x*pixels_per_block,                  y*pixels_per_block,
                                                  x*pixels_per_block+pixels_per_block, y*pixels_per_block+pixels_per_block) );
        }
    }

    std::stringstream ss;
    ss << "base_tiles/biome_" << chunkx << "_" << chunkz << ".png";
    image.write(ss.str());

    images[chunkx][chunkz] = image;

}

void BiomeImage::write(std::string filename) {
    Magick::InitializeMagick("./foo");

    int xcl, xch, zcl, zch;
    bool first = true;

    for(auto &i1: chunks) {
        for(auto &i2: i1.second) {
            if (first) {
                xcl = xch = i1.first;
                zcl = zch = i2.first;
                first = false;
            } else {
                xcl = std::min(xcl, i1.first);
                xch = std::max(xch, i1.first);
                zcl = std::min(zcl, i2.first);
                zch = std::max(zch, i2.first);
            }
        }
    }

    // in minecraft, y increases down. so -3 is above -2 and -2 is above -1.
    // this is the same order as the google xyz tiling scheme.
    // x has the usual euclidean order.

    // need to
    const int max_tile_level = std::ceil(log(std::max(xch-xcl, zch-zcl))/log(2));
    std::cout << "will need " << max_tile_level << "\n";

    // 16 blocks per chunk and 16 pixels per block.
    int width = pixels_per_block*16*(xch-xcl+1);
    int height = pixels_per_block*16*(zch-zcl+1);

    std::cout << "image size: " << width << " x " << height << "\n";
    std::cout << "block(chunk) bounds of the world " << 16*(xch-xcl+1) << "x" << 16*(zch-zcl+1);

    std::ofstream file;
    file.open("world.js");
    file << "const world_info = {\n";
    file << "  chunk_xl: " << xcl << ",\n";
    file << "  chunk_zl: " << zcl << ",\n";
    file << "  chunk_xh: " << xch << ",\n";
    file << "  chunk_zh: " << zch << ",\n";
    file << "  max_tile_level: " << max_tile_level << ",\n";
    file << "  tile_0_size: " << pow(2, max_tile_level)*16 << ",\n";

    file << "  eojson: 1\n";
    file << "}\n";
    file << "exports.info = world_info;\n";

    file.close();

    std::string tile_base = "tiling";
    boost::filesystem::create_directory(tile_base);

    std::string tile_dir = tile_base + "/" + boost::lexical_cast<std::string>(max_tile_level);
    boost::filesystem::create_directory(tile_dir);

    std::map<int, std::map<int, Magick::Image> > levelimages;
    for(auto &i1 : images) {
        int chunkx = i1.first;
        for(auto &i2 : i1.second) {
            int chunkz = i2.first;
            Magick::Image &image = i2.second;

            // map tiles begin at zero. shift the minecraft coords to starts at 0.
            int tilex = chunkx-xcl;
            int tilez = chunkz-zcl;

            std::stringstream ss;
            ss << tile_dir << "/" << "biome_" << tilex << "_" << tilez << ".png";
            image.write(ss.str());

            levelimages[tilex][tilez] = image;
        }
    }

    for(int level = max_tile_level-1; level>=0; level--) {
        std::string tile_dir = tile_base + "/" + boost::lexical_cast<std::string>(level);
        boost::filesystem::create_directory(tile_dir);

        std::map<int, std::map<int, Magick::Image > > levelimages_new;
        for(auto &i1 : levelimages) {
            int tilex = i1.first;
            for(auto &i2 : i1.second) {
                int tilez = i2.first;
                Magick::Image &image = i2.second;
                auto sbefore = image.size();

                int xnew = tilex/2;
                int znew = tilez/2;

                if (levelimages_new[xnew].find(znew) == levelimages_new[xnew].end()) {
                    levelimages_new[xnew][znew] = Magick::Image(Magick::Geometry(tile_size*2,tile_size*2), Magick::Color(0,0,0,0) );
                }
                Magick::Image &newimage = levelimages_new[xnew][znew];
                auto safter = newimage.size();

                newimage.composite(image, tile_size*(tilex%2), tile_size*(tilez%2), Magick::OverCompositeOp);
            }
        }


        // the & referencs in these iterations are very important because we are modifying (resize)
        // the images. by default, auto is a value, not a reference
        // https://stackoverflow.com/a/13934813/23630
        for(auto &i1 : levelimages_new) {
            int tilex = i1.first;
            for(auto &i2 : i1.second) {
                int tilez = i2.first;
                Magick::Image &image = i2.second;

                auto sbefore = image.size();
                image.resize(Magick::Geometry(tile_size,tile_size));
                auto safter = image.size();

                std::stringstream ss;
                ss << tile_dir << "/" << "biome_" << tilex << "_" << tilez << ".png";
                image.write(ss.str());
            }
        }

        levelimages = levelimages_new;
    }


    // not doing the big image
    return;


    Magick::Image image(Magick::Geometry(width, height), Magick::Color(0,0,0,0) );

    std::cout << "image size " << width << "," << height << "\n";
    for(auto i1:chunks) {
        int chunkx = i1.first;
        for(auto i2:i1.second) {
            int chunkz = i2.first;
            auto biomes = i2.second;
            //std::cout << "for chunk: " << chunkx << ", " << chunkz << "\n";
            for(int x=0; x<16; x++) {
                for(int z=0; z<16; z++) {
                    auto b = get_biome(biomes[x][z]);
                    Magick::Color c(QuantumRange/256*b.color.r,
                                    QuantumRange/256*b.color.g,
                                    QuantumRange/256*b.color.b,
                                    QuantumRange);
                    image.fillColor(c); // Fill color
                    //std::cout << "  biome at: " << (x+chunkx*16) << ", " << (z+chunkz*16) << " is ";
                    //std::cout << b.name << "\n";
                    int xl = (x+ (chunkx - xcl)*16) * pixels_per_block;
                    int zl = (z+ (chunkz - zcl)*16) * pixels_per_block;
                    int xh = xl+pixels_per_block;
                    int zh = zl+pixels_per_block;
                    //std::cout << "drawing " << xl << "," << zl << "," << xh << "," << zh;
                    //std::cout << " in color " << QuantumRange/256*b.color.r << "," << QuantumRange/256*b.color.g << ",";
                    //std::cout << QuantumRange/256*b.color.b << "\n";
                    image.draw( Magick::DrawableRectangle(xl, zl, xh, zh) );
                }
            }
        }
    }
    image.write(filename );
}
