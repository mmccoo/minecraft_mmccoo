# minecraft_mmccoo
some code for parsing and analyzing Minecraft Bedrock leveldb data. Check out this video showing what this code does:

[![demo](http://img.youtube.com/vi/S-5TawHSw1U/0.jpg)](http://www.youtube.com/watch?v=S-5TawHSw1U "Demo")


Setup instructions for Ubuntu (though my main machine is Arch):

# General packages you will need:
    sudo apt install npm
    sudo apt-get install libboost-all-dev
    sudo apt install cmake
    sudo apt-get install libcgal-dev
    sudo apt install build-essential

# setup Snappy
    git clone https://github.com/google/snappy.git
    cd snappy
    mkdir build
    mkdir install
    cd build
    cmake .. -DCMAKE_INSTALL_PREFIX=`realpath ../install`

# Magick++ needs to be installed if you want to generate tiles.
    // apt-file search Magick++.h
    // apt-file search Magick++-config
    sudo apt-get install libmagick++-6-headers libmagick++-6.q16-dev
 
# Compile leveldb-mcpe
    git clone https://github.com/Mojang/leveldb-mcpe.git
    cd leveldb-mcpe
    git apply ../minecraft_mmccoo/snappy_compressor.cc.patch
    git apply ../minecraft_mmccoo/table_test.cc.patch
    make
    
# Compile the code of this repo
First, compile the executeable

    make

# Now setup the web stuff

    cd web_stuff
    npm install
    npm run build

# Copy web files:
    cp -r web_stuff/dist/* /var/www/<your page dir>
    
# generate files for your world
    cd /var/www/<your page dir>
    
    // This will generate files in the map directory. The web htmls and js will look for that map directory
    minecraft_mmccoo/basic_test
