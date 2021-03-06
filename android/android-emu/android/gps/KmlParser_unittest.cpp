// Copyright (C) 2015 The Android Open Source Project
//
// This software is licensed under the terms of the GNU General Public
// License version 2, as published by the Free Software Foundation, and
// may be copied, distributed, and modified under those terms.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#include "android/gps/KmlParser.h"
#include "android/base/testing/TestTempDir.h"

#include "android/base/testing/Utils.h"

#include <fstream>
#include <gtest/gtest.h>

using android::base::TestTempDir;

namespace android_gps {

TEST(KmlParser, ParseNonexistentFile) {
    GpsFixArray locations;
    std::string error;
    ASSERT_FALSE(KmlParser::parseFile("", &locations, &error));
    ASSERT_EQ(0U, locations.size());
    EXPECT_EQ(std::string("KML document not parsed successfully."), error);
}

TEST(KmlParser, ParseEmptyFile) {
    {
        TestTempDir myDir("parse_location_tests");
        ASSERT_TRUE(myDir.path()); // NULL if error during creation.
        std::string path = myDir.makeSubPath("test.kml");

        std::ofstream myfile;
        myfile.open(path.c_str());
        myfile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
             "<kml xmlns=\"http://earth.google.com/kml/2.x\">"
             "</kml>";
        myfile.close();

        GpsFixArray locations;
        std::string error;
        ASSERT_TRUE(KmlParser::parseFile(path.c_str(), &locations, &error));
        EXPECT_EQ(0U, locations.size());
        EXPECT_EQ("", error);
     }   // destructor removes temp directory and all files under it.
}

TEST(KmlParser, ParseValidFile) {
    {
        TestTempDir myDir("parse_location_tests");
        ASSERT_TRUE(myDir.path()); // NULL if error during creation.
        std::string path = myDir.makeSubPath("test.kml");

        std::ofstream myfile;
        myfile.open(path.c_str());
        myfile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
         "<kml xmlns=\"http://earth.google.com/kml/2.x\">"
             "<Placemark>"
                 "<name>Simple placemark</name>"
                 "<description>Attached to the ground.</description>"
                 "<Point>"
                 "<coordinates>-122.0822035425683,37.42228990140251,0</coordinates>"
                 "</Point>"
             "</Placemark>"
         "</kml>";
        myfile.close();

        GpsFixArray locations;
        std::string error;
        ASSERT_TRUE(KmlParser::parseFile(path.c_str(), &locations, &error));
        EXPECT_EQ(1U, locations.size());
        EXPECT_EQ("", error);
     }   // destructor removes temp directory and all files under it.
}

TEST(KmlParser, ParseValidComplexFile) {
    {
        TestTempDir myDir("parse_location_tests");
        ASSERT_TRUE(myDir.path()); // NULL if error during creation.
        std::string path = myDir.makeSubPath("test.kml");

        std::ofstream myfile;
        myfile.open(path.c_str());
        myfile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
         "<kml xmlns=\"http://earth.google.com/kml/2.x\">"
         "<Document>"
             "<name>KML Samples</name>"
             "<Style id=\"globeIcon\">"
             "<IconStyle></IconStyle><LineStyle><width>2</width></LineStyle>"
             "</Style>"
             "<Folder>"
                 "<name>Placemarks</name>"
                 "<description>These are just some</description>"
                 "<LookAt>"
                 "<tilt>40.5575073395506</tilt><range>500.6566641072245</range>"
                 "</LookAt>"
                 "<Placemark>"
                     "<name>Tessellated</name>"
                     "<visibility>0</visibility>"
                     "<description>Black line (10 pixels wide), height tracks terrain</description>"
                     "<LookAt><longitude>-122.0839597145766</longitude></LookAt>"
                     "<styleUrl>#downArrowIcon</styleUrl>"
                     "<Point>"
                         "<altitudeMode>relativeToGround</altitudeMode>"
                         "<coordinates>-122.084075,37.4220033612141,50</coordinates>"
                     "</Point>"
                 "</Placemark>"
                 "<Placemark>"
                     "<name>Transparent</name>"
                     "<visibility>0</visibility>"
                     "<styleUrl>#transRedPoly</styleUrl>"
                     "<Polygon>"
                         "<extrude>1</extrude>"
                         "<altitudeMode>relativeToGround</altitudeMode>"
                         "<outerBoundaryIs>"
                         "<LinearRing>"
                         "<coordinates> -122.084075,37.4220033612141,50</coordinates>"
                         "</LinearRing>"
                         "</outerBoundaryIs>"
                     "</Polygon>"
                 "</Placemark>"
             "</Folder>"
             "<Placemark>"
                 "<name>Fruity</name>"
                 "<visibility>0</visibility>"
                 "<description><![CDATA[If the <tessellate> tag has a value of n]]></description>"
                 "<LookAt><longitude>-112.0822680013139</longitude></LookAt>"
                 "<LineString>"
                     "<tessellate>1</tessellate>"
                     "<coordinates> -122.084075,37.4220033612141,50 </coordinates>"
                 "</LineString>"
             "</Placemark>"
         "</Document>"
         "</kml>";
        myfile.close();

        GpsFixArray locations;
        std::string error;
        ASSERT_TRUE(KmlParser::parseFile(path.c_str(), &locations, &error));
        EXPECT_EQ("", error);
        EXPECT_EQ(3U, locations.size());

        EXPECT_EQ("Tessellated", locations[0].name);
        EXPECT_EQ("Black line (10 pixels wide), height tracks terrain", locations[0].description);
        EXPECT_EQ("Transparent", locations[1].name);
        EXPECT_EQ("", locations[1].description);
        EXPECT_EQ("Fruity", locations[2].name);
        EXPECT_EQ("If the <tessellate> tag has a value of n", locations[2].description);

        for (unsigned i = 0; i < locations.size(); ++i) {
            EXPECT_FLOAT_EQ(-122.084075, locations[i].longitude);
            EXPECT_FLOAT_EQ(37.4220033612141, locations[i].latitude);
            EXPECT_FLOAT_EQ(50, locations[i].elevation);
        }
    }   // destructor removes temp directory and all files under it.
}

TEST(KmlParser, ParseOneCoordinate) {
        TestTempDir myDir("parse_location_tests");
        ASSERT_TRUE(myDir.path()); // NULL if error during creation.
        std::string path = myDir.makeSubPath("test.kml");

        std::ofstream myfile;
        myfile.open(path.c_str());
        myfile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<kml xmlns=\"http://earth.google.com/kml/2.x\">"
            "<Placemark>"
                    "<Point>"
                    "<coordinates>-122.0822035425683,37.42228990140251,0</coordinates>"
                    "</Point>"
                "</Placemark>"
            "</kml>";
        myfile.close();

        GpsFixArray locations;
        std::string error;

        ASSERT_TRUE(KmlParser::parseFile(path.c_str(), &locations, &error));
        EXPECT_EQ("", error);
        EXPECT_EQ(1U, locations.size());
        EXPECT_FLOAT_EQ(-122.0822035425683, locations[0].longitude);
        EXPECT_FLOAT_EQ(37.42228990140251, locations[0].latitude);
        EXPECT_FLOAT_EQ(0.0, locations[0].elevation);
}

TEST(KmlParser, ParseMultipleCoordinates) {
        TestTempDir myDir("parse_location_tests");
        ASSERT_TRUE(myDir.path()); // NULL if error during creation.
        std::string path = myDir.makeSubPath("test.kml");

        std::ofstream myfile;
        myfile.open(path.c_str());
        myfile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<kml xmlns=\"http://earth.google.com/kml/2.x\">"
            "<Placemark>"
                    "<LineString>"
                    "<coordinates>-122.0822035425683,37.42228990140251,0 \
                    10.4,39.,20\t\t0,21.4,1"
                    "</coordinates>"
                    "</LineString>"
            "</Placemark>"
            "</kml>";;
        myfile.close();

        GpsFixArray locations;
        std::string error;

        ASSERT_TRUE(KmlParser::parseFile(path.c_str(), &locations, &error));
        EXPECT_EQ("", error);
        ASSERT_EQ(3U, locations.size());

        EXPECT_FLOAT_EQ(-122.0822035425683, locations[0].longitude);
        EXPECT_FLOAT_EQ(37.42228990140251, locations[0].latitude);
        EXPECT_FLOAT_EQ(0, locations[0].elevation);
        EXPECT_FLOAT_EQ(10.4, locations[1].longitude);
        EXPECT_FLOAT_EQ(39., locations[1].latitude);
        EXPECT_FLOAT_EQ(20, locations[1].elevation);
        EXPECT_FLOAT_EQ(0, locations[2].longitude);
        EXPECT_FLOAT_EQ(21.4, locations[2].latitude);
        EXPECT_FLOAT_EQ(1, locations[2].elevation);
}

TEST(KmlParser, ParseBadCoordinates) {
    TestTempDir myDir("parse_location_tests");
    ASSERT_TRUE(myDir.path()); // NULL if error during creation.
    std::string path = myDir.makeSubPath("test.kml");

    std::ofstream myfile;
    myfile.open(path.c_str());
    // Invalid because of internal spaces.
    myfile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<kml xmlns=\"http://earth.google.com/kml/2.x\">"
            "<Placemark>"
                    "<LineString>"
                    "<coordinates>-122.0822035425683, 37.42228990140251, 0 \
                    10.4,39.20\t021.41"
                    "</coordinates>"
                    "</LineString>"
            "</Placemark>"
            "</kml>";
    myfile.close();

    GpsFixArray locations;
    std::string error;

    ASSERT_FALSE(KmlParser::parseFile(path.c_str(), &locations, &error));
}

TEST(KmlParser, ParseLocationNormal) {
    TestTempDir myDir("parse_location_tests");
    ASSERT_TRUE(myDir.path()); // NULL if error during creation.
    std::string path = myDir.makeSubPath("test.kml");

    std::ofstream myfile;
    myfile.open(path.c_str());
    myfile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<kml xmlns=\"http://earth.google.com/kml/2.x\">"
            "<Placemark>"
                    "<name>Simple placemark</name>"
                    "<description>Attached to the ground.</description>"
                    "<Point>"
                    "<coordinates>-122.0822035425683,37.42228990140251,0</coordinates>"
                    "</Point>"
            "</Placemark>"
            "</kml>";
    myfile.close();

    GpsFixArray locations;
    std::string error;

    ASSERT_TRUE(KmlParser::parseFile(path.c_str(), &locations, &error));

    for (unsigned i = 0; i < locations.size(); ++i) {
        EXPECT_EQ("Simple placemark", locations[i].name);
        EXPECT_EQ("Attached to the ground.", locations[i].description);
        EXPECT_FLOAT_EQ(-122.0822035425683, locations[i].longitude);
        EXPECT_FLOAT_EQ(37.42228990140251, locations[i].latitude);
        EXPECT_FLOAT_EQ(0, locations[i].elevation);
    }
}

TEST(KmlParser, ParseLocationNormalMissingOptionalFields) {
    TestTempDir myDir("parse_location_tests");
    ASSERT_TRUE(myDir.path()); // NULL if error during creation.
    std::string path = myDir.makeSubPath("test.kml");

    std::ofstream myfile;
    myfile.open(path.c_str());
    myfile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<kml xmlns=\"http://earth.google.com/kml/2.x\">"
            "<Placemark>"
                    "<Point>"
                    "<coordinates>-122.0822035425683,37.42228990140251,0</coordinates>"
                    "</Point>"
            "</Placemark>"
            "</kml>";
    myfile.close();

    GpsFixArray locations;
    std::string error;

    ASSERT_TRUE(KmlParser::parseFile(path.c_str(), &locations, &error));
    EXPECT_EQ("", error);
    ASSERT_EQ(1U, locations.size());

    for (unsigned i = 0; i < locations.size(); ++i) {
        EXPECT_EQ("", locations[i].name);
        EXPECT_EQ("", locations[i].description);
        EXPECT_FLOAT_EQ(-122.0822035425683, locations[i].longitude);
        EXPECT_FLOAT_EQ(37.42228990140251, locations[i].latitude);
        EXPECT_FLOAT_EQ(0, locations[i].elevation);
    }
}

TEST(KmlParser, ParseLocationMissingRequiredFields) {
    TestTempDir myDir("parse_location_tests");
    ASSERT_TRUE(myDir.path()); // NULL if error during creation.
    std::string path = myDir.makeSubPath("test.kml");

    std::ofstream myfile;
    myfile.open(path.c_str());
    myfile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<kml xmlns=\"http://earth.google.com/kml/2.x\">"
            "<Placemark>"
                    "<name>Simple placemark</name>"
                    "<description>Attached to the ground.</description>"
            "</Placemark>"
            "</kml>";
    myfile.close();

    GpsFixArray locations;
    std::string error;

    ASSERT_FALSE(KmlParser::parseFile(path.c_str(), &locations, &error));
    EXPECT_EQ("Location found with missing or malformed coordinates", error);
}

TEST(KmlParser, ParseLocationNameOnlyFirst) {
    TestTempDir myDir("parse_location_tests");
    ASSERT_TRUE(myDir.path()); // NULL if error during creation.
    std::string path = myDir.makeSubPath("test.kml");

    std::ofstream myfile;
    myfile.open(path.c_str());
    myfile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<kml xmlns=\"http://earth.google.com/kml/2.x\">"
            "<Placemark>"
                    "<name>Simple placemark</name>"
                    "<description>Attached to the ground.</description>"
                    "<LineString>"
                    "<coordinates>-122.0822035425683,37.42228990140251,0\
                    -122.0822035425683,37.42228990140251,0</coordinates>"
                    "</LineString>"
            "</Placemark>"
            "</kml>";
    myfile.close();

    GpsFixArray locations;
    std::string error;
    EXPECT_TRUE(KmlParser::parseFile(path.c_str(), &locations, &error));
    EXPECT_EQ("", error);
    ASSERT_EQ(2U, locations.size());

    EXPECT_EQ("Simple placemark", locations[0].name);
    EXPECT_EQ("Attached to the ground.", locations[0].description);
    EXPECT_EQ("", locations[1].name);
    EXPECT_EQ("", locations[1].description);

    for (unsigned i = 0; i < locations.size(); ++i) {
        EXPECT_FLOAT_EQ(-122.0822035425683, locations[i].longitude);
        EXPECT_FLOAT_EQ(37.42228990140251, locations[i].latitude);
        EXPECT_FLOAT_EQ(0, locations[i].elevation);
    }
}

TEST(KmlParser, ParseMultipleLocations) {
     TestTempDir myDir("parse_location_tests");
    ASSERT_TRUE(myDir.path()); // NULL if error during creation.
    std::string path = myDir.makeSubPath("test.kml");

    std::ofstream myfile;
    myfile.open(path.c_str());
    myfile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<kml xmlns=\"http://earth.google.com/kml/2.x\">"
            "<Placemark>"
                    "<name>Simple placemark</name>"
                    "<description>Attached to the ground.</description>"
                    "<Point>"
                    "<coordinates>-122.0822035425683,37.42228990140251,0</coordinates>"
                    "</Point>"
            "</Placemark>"
            "<Placemark>"
                    "<name>Simple placemark</name>"
                    "<description>Attached to the ground.</description>"
                    "<Point>"
                    "<coordinates>-122.0822035425683,37.42228990140251,0\
                    -122.0822035425683,37.42228990140251,0</coordinates>"
                    "</Point>"
            "</Placemark>"
            "<Placemark>"
                    "<name>Simple placemark</name>"
                    "<description>Attached to the ground.</description>"
                    "<Point>"
                    "<coordinates>-122.0822035425683,37.42228990140251,0</coordinates>"
                    "</Point>"
            "</Placemark>"
            "</kml>";
    myfile.close();

    GpsFixArray locations;
    std::string error;
    EXPECT_TRUE(KmlParser::parseFile(path.c_str(), &locations, &error));
    EXPECT_EQ("", error);
    ASSERT_EQ(4U, locations.size());

    for (unsigned i = 0; i < locations.size(); ++i) {
        if (i != 2) {
            EXPECT_EQ("Simple placemark", locations[i].name);
            EXPECT_EQ("Attached to the ground.", locations[i].description);
        } else {
            EXPECT_EQ("", locations[i].name);
            EXPECT_EQ("", locations[i].description);
        }
        EXPECT_FLOAT_EQ(-122.0822035425683, locations[i].longitude);
        EXPECT_FLOAT_EQ(37.42228990140251, locations[i].latitude);
        EXPECT_FLOAT_EQ(0, locations[i].elevation);
    }
}

TEST(KmlParser, TraverseEmptyDoc) {
    TestTempDir myDir("parse_location_tests");
    ASSERT_TRUE(myDir.path()); // NULL if error during creation.
    std::string path = myDir.makeSubPath("test.kml");

    std::ofstream myfile;
    myfile.open(path.c_str());
    myfile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<kml xmlns=\"http://earth.google.com/kml/2.x\"></kml>";
    myfile.close();

    GpsFixArray locations;
    std::string error;
    EXPECT_TRUE(KmlParser::parseFile(path.c_str(), &locations, &error));
    EXPECT_EQ("", error);
    EXPECT_EQ(0U, locations.size());
}

TEST(KmlParser, TraverseDocNoPlacemarks) {
    TestTempDir myDir("parse_location_tests");
    ASSERT_TRUE(myDir.path()); // NULL if error during creation.
    std::string path = myDir.makeSubPath("test.kml");

    std::ofstream myfile;
    myfile.open(path.c_str());
    myfile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<kml xmlns=\"http://earth.google.com/kml/2.x\">"
        "<LineString></LineString>"
        "<name></name>"
        "</kml>";
    myfile.close();

    GpsFixArray locations;
    std::string error;
    EXPECT_TRUE(KmlParser::parseFile(path.c_str(), &locations, &error));
    EXPECT_EQ("", error);
    EXPECT_EQ(0U, locations.size());
}

TEST(KmlParser, TraverseNestedDoc) {
    TestTempDir myDir("parse_location_tests");
    ASSERT_TRUE(myDir.path()); // NULL if error during creation.
    std::string path = myDir.makeSubPath("test.kml");

    std::ofstream myfile;
    myfile.open(path.c_str());
    myfile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<kml xmlns=\"http://earth.google.com/kml/2.x\">"
            "<Document>"
            "<Folder>"
                    "<name>Placemarks</name>"
                    "<description>These are just some of the different kinds of placemarks with"
                    "which you can mark your favorite places</description>"
                    "<LookAt>"
                            "<longitude>-122.0839597145766</longitude>"
                            "<latitude>37.42222904525232</latitude>"
                            "<altitude>0</altitude>"
                            "<heading>-148.4122922628044</heading>"
                            "<tilt>40.5575073395506</tilt>"
                            "<range>500.6566641072245</range>"
                    "</LookAt>"
                    "<Placemark>"
                            "<name>Simple placemark</name>"
                            "<description>Attached to the ground.</description>"
                            "<Point>"
                            "<coordinates>-122.0822035425683,37.42228990140251,0</coordinates>"
                            "</Point>"
                    "</Placemark>"
            "</Folder>"
            "</Document>"
            "</kml>";
    myfile.close();

    GpsFixArray locations;
    std::string error;
    EXPECT_TRUE(KmlParser::parseFile(path.c_str(), &locations, &error));
    EXPECT_EQ("", error);
    ASSERT_EQ(1U, locations.size());

    EXPECT_EQ("Simple placemark", locations[0].name);
    EXPECT_EQ("Attached to the ground.", locations[0].description);
    EXPECT_FLOAT_EQ(-122.0822035425683, locations[0].longitude);
    EXPECT_FLOAT_EQ(37.42228990140251, locations[0].latitude);
    EXPECT_FLOAT_EQ(0, locations[0].elevation);
}

TEST(KmlParser, ParsePlacemarkNullNameNoCrash) {
    TestTempDir myDir("parse_location_tests");
    ASSERT_TRUE(myDir.path()); // NULL if error during creation.
    std::string path = myDir.makeSubPath("test.kml");

    std::ofstream myfile;
    myfile.open(path.c_str());
    myfile <<
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<kml xmlns=\"http://earth.google.com/kml/2.x\">"
            "<Placemark>"
            "<name/>"
            "<description/>"
            "<Point>"
            "<coordinates>-122.0822035425683,37.42228990140251,0</coordinates>"
            "</Point>"
            "</Placemark>"
            "</kml>";
    myfile.close();

    GpsFixArray locations;
    std::string error;
    EXPECT_TRUE(KmlParser::parseFile(path.c_str(), &locations, &error));
    ASSERT_EQ(1U, locations.size());
    EXPECT_STREQ("", locations.front().name.c_str());
    EXPECT_STREQ("", locations.front().description.c_str());
}

// Flaky test; uses locale.
TEST(KmlParser, DISABLED_ParseLocationNormalCommaLocale) {
    auto scopedCommaLocale = setScopedCommaLocale();

    TestTempDir myDir("parse_location_tests");
    ASSERT_TRUE(myDir.path()); // NULL if error during creation.
    std::string path = myDir.makeSubPath("test.kml");

    std::ofstream myfile;
    myfile.open(path.c_str());
    myfile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<kml xmlns=\"http://earth.google.com/kml/2.x\">"
            "<Placemark>"
                    "<name>Simple placemark</name>"
                    "<description>Attached to the ground.</description>"
                    "<Point>"
                    "<coordinates>-122.0822035425683,37.42228990140251,0</coordinates>"
                    "</Point>"
            "</Placemark>"
            "</kml>";
    myfile.close();

    GpsFixArray locations;
    std::string error;

    ASSERT_TRUE(KmlParser::parseFile(path.c_str(), &locations, &error));

    for (unsigned i = 0; i < locations.size(); ++i) {
        EXPECT_EQ("Simple placemark", locations[i].name);
        EXPECT_EQ("Attached to the ground.", locations[i].description);
        EXPECT_FLOAT_EQ(-122.0822035425683, locations[i].longitude);
        EXPECT_FLOAT_EQ(37.42228990140251, locations[i].latitude);
        EXPECT_FLOAT_EQ(0, locations[i].elevation);
    }
}
}
