


var __extends = (this && this.__extends) || (function () {
    var extendStatics = function (d, b) {
        extendStatics = Object.setPrototypeOf ||
            ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
            function (d, b) { for (var p in b) if (b.hasOwnProperty(p)) d[p] = b[p]; };
        return extendStatics(d, b);
    };
    return function (d, b) {
        extendStatics(d, b);
        function __() { this.constructor = d; }
        d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
    };
})();


import { assert } from 'ol/asserts.js';
import Feature from 'ol/Feature.js';
import { transformGeometryWithOptions } from 'ol/format/Feature.js';
import JSONFeature from 'ol/format/JSONFeature.js';
import GeometryCollection from 'ol/geom/GeometryCollection.js';
import LineString from 'ol/geom/LineString.js';
import MultiLineString from 'ol/geom/MultiLineString.js';
import MultiPoint from 'ol/geom/MultiPoint.js';
import MultiPolygon from 'ol/geom/MultiPolygon.js';
import Point from 'ol/geom/Point.js';
import Polygon from 'ol/geom/Polygon.js';
import { assign, isEmpty } from 'ol/obj.js';
import { get as getProjection } from 'ol/proj.js';
import GeometryType from 'ol/geom/GeometryType.js';



var MyGeoJSON = /** @class */ (function (_super) {
    __extends(MyGeoJSON, _super);


    function MyGeoJSON(yl, yh, opt_options) {
        var _this = this;
        var options = opt_options ? opt_options : {};
        _this = _super.call(this) || this;
        /**
         * @inheritDoc
         */
        _this.dataProjection = getProjection(options.dataProjection ?
            options.dataProjection : 'EPSG:4326');
        if (options.featureProjection) {
            _this.defaultFeatureProjection = getProjection(options.featureProjection);
        }
        /**
         * Name of the geometry attribute for features.
         * @type {string|undefined}
         * @private
         */
        _this.geometryName_ = options.geometryName;
        /**
         * Look for the geometry name in the feature GeoJSON
         * @type {boolean|undefined}
         * @private
         */
        _this.extractGeometryName_ = options.extractGeometryName;

        _this.yl = yl;
        _this.yh = yh;
        return _this;
    }
    /**
     * @inheritDoc
     */
    MyGeoJSON.prototype.readFeatureFromObject = function (object, opt_options) {
        /**
         * @type {MyGeoJSONFeature}
         */
        var geoJSONFeature = null;
        if (object['type'] === 'Feature') {
            geoJSONFeature = /** @type {MyGeoJSONFeature} */ (object);
        }
        else {
            geoJSONFeature = {
                'type': 'Feature',
                'geometry': /** @type {MyGeoJSONGeometry} */ (object),
                'properties': null
            };
        }
        var geometry = readGeometry(geoJSONFeature['geometry'], opt_options, this.yl, this.yh);
        var feature = new Feature();
        if (this.geometryName_) {
            feature.setGeometryName(this.geometryName_);
        }
        else if (this.extractGeometryName_ && 'geometry_name' in geoJSONFeature !== undefined) {
            feature.setGeometryName(geoJSONFeature['geometry_name']);
        }
        feature.setGeometry(geometry);
        if ('id' in geoJSONFeature) {
            feature.setId(geoJSONFeature['id']);
        }
        if (geoJSONFeature['properties']) {
            feature.setProperties(geoJSONFeature['properties'], true);
        }
        return feature;
    };
    /**
     * @inheritDoc
     */
    MyGeoJSON.prototype.readFeaturesFromObject = function (object, opt_options) {
        var geoJSONObject = /** @type {MyGeoJSONObject} */ (object);
        /** @type {Array<import("../Feature.js").default>} */
        var features = null;
        if (geoJSONObject['type'] === 'FeatureCollection') {
            var geoJSONFeatureCollection = /** @type {MyGeoJSONFeatureCollection} */ (object);
            features = [];
            var geoJSONFeatures = geoJSONFeatureCollection['features'];
            for (var i = 0, ii = geoJSONFeatures.length; i < ii; ++i) {
                features.push(this.readFeatureFromObject(geoJSONFeatures[i], opt_options));
            }
        }
        else {
            features = [this.readFeatureFromObject(object, opt_options)];
        }
        return features;
    };
    /**
     * @inheritDoc
     */
    MyGeoJSON.prototype.readGeometryFromObject = function (object, opt_options, yl, yh) {
        return readGeometry(/** @type {MyGeoJSONGeometry} */ (object), opt_options, yl, yh);
    };
    /**
     * @inheritDoc
     */
    MyGeoJSON.prototype.readProjectionFromObject = function (object) {
        var crs = object['crs'];
        var projection;
        if (crs) {
            if (crs['type'] == 'name') {
                projection = getProjection(crs['properties']['name']);
            }
            else {
                assert(false, 36); // Unknown SRS type
            }
        }
        else {
            projection = this.dataProjection;
        }
        return (
        /** @type {import("../proj/Projection.js").default} */ (projection));
    };
    /**
     * Encode a feature as a MyGeoJSON Feature object.
     *
     * @param {import("../Feature.js").default} feature Feature.
     * @param {import("./Feature.js").WriteOptions=} opt_options Write options.
     * @return {MyGeoJSONFeature} Object.
     * @override
     * @api
     */
    MyGeoJSON.prototype.writeFeatureObject = function (feature, opt_options) {
        opt_options = this.adaptOptions(opt_options);
        /** @type {MyGeoJSONFeature} */
        var object = {
            'type': 'Feature',
            geometry: null,
            properties: null
        };
        var id = feature.getId();
        if (id !== undefined) {
            object.id = id;
        }
        var geometry = feature.getGeometry();
        if (geometry) {
            object.geometry = writeGeometry(geometry, opt_options);
        }
        var properties = feature.getProperties();
        delete properties[feature.getGeometryName()];
        if (!isEmpty(properties)) {
            object.properties = properties;
        }
        return object;
    };
    /**
     * Encode an array of features as a MyGeoJSON object.
     *
     * @param {Array<import("../Feature.js").default>} features Features.
     * @param {import("./Feature.js").WriteOptions=} opt_options Write options.
     * @return {MyGeoJSONFeatureCollection} MyGeoJSON Object.
     * @override
     * @api
     */
    MyGeoJSON.prototype.writeFeaturesObject = function (features, opt_options) {
        opt_options = this.adaptOptions(opt_options);
        var objects = [];
        for (var i = 0, ii = features.length; i < ii; ++i) {
            objects.push(this.writeFeatureObject(features[i], opt_options));
        }
        return {
            type: 'FeatureCollection',
            features: objects
        };
    };
    /**
     * Encode a geometry as a MyGeoJSON object.
     *
     * @param {import("../geom/Geometry.js").default} geometry Geometry.
     * @param {import("./Feature.js").WriteOptions=} opt_options Write options.
     * @return {MyGeoJSONGeometry|MyGeoJSONGeometryCollection} Object.
     * @override
     * @api
     */
    MyGeoJSON.prototype.writeGeometryObject = function (geometry, opt_options) {
        return writeGeometry(geometry, this.adaptOptions(opt_options));
    };
    return MyGeoJSON;
}(JSONFeature));
/**
 * @param {MyGeoJSONGeometry|MyGeoJSONGeometryCollection} object Object.
 * @param {import("./Feature.js").ReadOptions=} opt_options Read options.
 * @return {import("../geom/Geometry.js").default} Geometry.
 */
function readGeometry(object, opt_options, yl, yh) {
    if (!object) {
        return null;
    }

    if (('coordinates' in object) && (object['coordinates'].length > 0)) {
        let coords = object['coordinates']
        
        // this is for things like polygons. coordinates has a bunch of 
        // lists of pairs. the first one is the boundary and the ones after 
        // are holes.
        if (Array.isArray(coords[0])) {
            for (let poly of coords) {
                for (let pt of poly) {
                    pt[1] = yl+yh-pt[1];
                }
            }
        } else {
            coords[1] = yl+yh-coords[1];
        }        
    }

    /**
     * @type {import("../geom/Geometry.js").default}
     */
    var geometry;
    switch (object['type']) {
        case GeometryType.POINT: {
            geometry = readPointGeometry(/** @type {MyGeoJSONPoint} */ (object));
            break;
        }
        case GeometryType.LINE_STRING: {
            geometry = readLineStringGeometry(/** @type {MyGeoJSONLineString} */ (object));
            break;
        }
        case GeometryType.POLYGON: {
            geometry = readPolygonGeometry(/** @type {MyGeoJSONPolygon} */ (object));
            break;
        }
        case GeometryType.MULTI_POINT: {
            geometry = readMultiPointGeometry(/** @type {MyGeoJSONMultiPoint} */ (object));
            break;
        }
        case GeometryType.MULTI_LINE_STRING: {
            geometry = readMultiLineStringGeometry(/** @type {MyGeoJSONMultiLineString} */ (object));
            break;
        }
        case GeometryType.MULTI_POLYGON: {
            geometry = readMultiPolygonGeometry(/** @type {MyGeoJSONMultiPolygon} */ (object));
            break;
        }
        case GeometryType.GEOMETRY_COLLECTION: {
            geometry = readGeometryCollectionGeometry(/** @type {MyGeoJSONGeometryCollection} */ (object));
            break;
        }
        default: {
            throw new Error('Unsupported MyGeoJSON type: ' + object.type);
        }
    }
    return transformGeometryWithOptions(geometry, false, opt_options);
}
/**
 * @param {MyGeoJSONGeometryCollection} object Object.
 * @param {import("./Feature.js").ReadOptions=} opt_options Read options.
 * @return {GeometryCollection} Geometry collection.
 */
function readGeometryCollectionGeometry(object, opt_options) {
    var geometries = object['geometries'].map(
    /**
     * @param {MyGeoJSONGeometry} geometry Geometry.
     * @return {import("../geom/Geometry.js").default} geometry Geometry.
     */
    function (geometry) {
        return readGeometry(geometry, opt_options);
    });
    return new GeometryCollection(geometries);
}
/**
 * @param {MyGeoJSONPoint} object Object.
 * @return {Point} Point.
 */
function readPointGeometry(object) {
    return new Point(object['coordinates']);
}
/**
 * @param {MyGeoJSONLineString} object Object.
 * @return {LineString} LineString.
 */
function readLineStringGeometry(object) {
    return new LineString(object['coordinates']);
}
/**
 * @param {MyGeoJSONMultiLineString} object Object.
 * @return {MultiLineString} MultiLineString.
 */
function readMultiLineStringGeometry(object) {
    return new MultiLineString(object['coordinates']);
}
/**
 * @param {MyGeoJSONMultiPoint} object Object.
 * @return {MultiPoint} MultiPoint.
 */
function readMultiPointGeometry(object) {
    return new MultiPoint(object['coordinates']);
}
/**
 * @param {MyGeoJSONMultiPolygon} object Object.
 * @return {MultiPolygon} MultiPolygon.
 */
function readMultiPolygonGeometry(object) {
    return new MultiPolygon(object['coordinates']);
}
/**
 * @param {MyGeoJSONPolygon} object Object.
 * @return {Polygon} Polygon.
 */
function readPolygonGeometry(object) {
    return new Polygon(object['coordinates']);
}
/**
 * @param {import("../geom/Geometry.js").default} geometry Geometry.
 * @param {import("./Feature.js").WriteOptions=} opt_options Write options.
 * @return {MyGeoJSONGeometry} MyGeoJSON geometry.
 */
function writeGeometry(geometry, opt_options) {
    geometry = transformGeometryWithOptions(geometry, true, opt_options);
    var type = geometry.getType();
    /** @type {MyGeoJSONGeometry} */
    var geoJSON;
    switch (type) {
        case GeometryType.POINT: {
            geoJSON = writePointGeometry(/** @type {Point} */ (geometry), opt_options);
            break;
        }
        case GeometryType.LINE_STRING: {
            geoJSON = writeLineStringGeometry(/** @type {LineString} */ (geometry), opt_options);
            break;
        }
        case GeometryType.POLYGON: {
            geoJSON = writePolygonGeometry(/** @type {Polygon} */ (geometry), opt_options);
            break;
        }
        case GeometryType.MULTI_POINT: {
            geoJSON = writeMultiPointGeometry(/** @type {MultiPoint} */ (geometry), opt_options);
            break;
        }
        case GeometryType.MULTI_LINE_STRING: {
            geoJSON = writeMultiLineStringGeometry(/** @type {MultiLineString} */ (geometry), opt_options);
            break;
        }
        case GeometryType.MULTI_POLYGON: {
            geoJSON = writeMultiPolygonGeometry(/** @type {MultiPolygon} */ (geometry), opt_options);
            break;
        }
        case GeometryType.GEOMETRY_COLLECTION: {
            geoJSON = writeGeometryCollectionGeometry(/** @type {GeometryCollection} */ (geometry), opt_options);
            break;
        }
        case GeometryType.CIRCLE: {
            geoJSON = {
                type: 'GeometryCollection',
                geometries: []
            };
            break;
        }
        default: {
            throw new Error('Unsupported geometry type: ' + type);
        }
    }
    return geoJSON;
}
/**
 * @param {GeometryCollection} geometry Geometry.
 * @param {import("./Feature.js").WriteOptions=} opt_options Write options.
 * @return {MyGeoJSONGeometryCollection} MyGeoJSON geometry collection.
 */
function writeGeometryCollectionGeometry(geometry, opt_options) {
    var geometries = geometry.getGeometriesArray().map(function (geometry) {
        var options = assign({}, opt_options);
        delete options.featureProjection;
        return writeGeometry(geometry, options);
    });
    return {
        type: 'GeometryCollection',
        geometries: geometries
    };
}
/**
 * @param {LineString} geometry Geometry.
 * @param {import("./Feature.js").WriteOptions=} opt_options Write options.
 * @return {MyGeoJSONGeometry} MyGeoJSON geometry.
 */
function writeLineStringGeometry(geometry, opt_options) {
    return {
        type: 'LineString',
        coordinates: geometry.getCoordinates()
    };
}
/**
 * @param {MultiLineString} geometry Geometry.
 * @param {import("./Feature.js").WriteOptions=} opt_options Write options.
 * @return {MyGeoJSONGeometry} MyGeoJSON geometry.
 */
function writeMultiLineStringGeometry(geometry, opt_options) {
    return {
        type: 'MultiLineString',
        coordinates: geometry.getCoordinates()
    };
}
/**
 * @param {MultiPoint} geometry Geometry.
 * @param {import("./Feature.js").WriteOptions=} opt_options Write options.
 * @return {MyGeoJSONGeometry} MyGeoJSON geometry.
 */
function writeMultiPointGeometry(geometry, opt_options) {
    return {
        type: 'MultiPoint',
        coordinates: geometry.getCoordinates()
    };
}
/**
 * @param {MultiPolygon} geometry Geometry.
 * @param {import("./Feature.js").WriteOptions=} opt_options Write options.
 * @return {MyGeoJSONGeometry} MyGeoJSON geometry.
 */
function writeMultiPolygonGeometry(geometry, opt_options) {
    var right;
    if (opt_options) {
        right = opt_options.rightHanded;
    }
    return {
        type: 'MultiPolygon',
        coordinates: geometry.getCoordinates(right)
    };
}
/**
 * @param {Point} geometry Geometry.
 * @param {import("./Feature.js").WriteOptions=} opt_options Write options.
 * @return {MyGeoJSONGeometry} MyGeoJSON geometry.
 */
function writePointGeometry(geometry, opt_options) {
    return {
        type: 'Point',
        coordinates: geometry.getCoordinates()
    };
}
/**
 * @param {Polygon} geometry Geometry.
 * @param {import("./Feature.js").WriteOptions=} opt_options Write options.
 * @return {MyGeoJSONGeometry} MyGeoJSON geometry.
 */
function writePolygonGeometry(geometry, opt_options) {
    var right;
    if (opt_options) {
        right = opt_options.rightHanded;
    }
    return {
        type: 'Polygon',
        coordinates: geometry.getCoordinates(right)
    };
}
export default MyGeoJSON;
//# sourceMappingURL=MyGeoJSON.js.map
