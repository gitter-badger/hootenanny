/*
 * This file is part of Hootenanny.
 *
 * Hootenanny is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * --------------------------------------------------------------------
 *
 * The following copyright notices are generated automatically. If you
 * have a new notice to add, please use the format:
 * " * @copyright Copyright ..."
 * This will properly maintain the copyright information. DigitalGlobe
 * copyrights will be updated automatically.
 *
 * @copyright Copyright (C) 2012, 2013, 2014 DigitalGlobe (http://www.digitalglobe.com/)
 */

// Hoot
#include <hoot/core/schema/OsmSchema.h>
#include <hoot/core/schema/JsonSchemaLoader.h>
#include <hoot/core/util/ConfPath.h>
#include <hoot/core/util/Log.h>

using namespace hoot;

// CPP Unit
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>

// Qt
#include <QDebug>
#include <QFile>

#include "../TestUtils.h"

const double epsilon = 1e-6;

namespace hoot
{

class OsmSchemaTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(OsmSchemaTest);
  CPPUNIT_TEST(averageTest);
  CPPUNIT_TEST(categoryTest);
  CPPUNIT_TEST(loadTest);
  CPPUNIT_TEST(commonAncestorTest);
  CPPUNIT_TEST(distanceTest);
  CPPUNIT_TEST(getChildTagsTest);
  CPPUNIT_TEST(getTagTest);
  CPPUNIT_TEST(isAncestorTest);
  CPPUNIT_TEST(isAreaTest);
  CPPUNIT_TEST(isMetaDataTest);
  CPPUNIT_TEST(religionTest);
  CPPUNIT_TEST_SUITE_END();

public:

  /**
   * Test calculating the average between two tags with weights.
   */
  void averageTest()
  {
    OsmSchema uut;

    uut.createTestingGraph();

    double score;
    QString avg;
    avg = uut.average("highway=primary", "highway=residential", score);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.64, score, 0.001);
    CPPUNIT_ASSERT_EQUAL(avg.toStdString(), std::string("highway=secondary"));

    avg = uut.average("highway=Primary", "Highway=residential", score);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.64, score, 0.001);
    CPPUNIT_ASSERT_EQUAL(avg.toStdString(), std::string("highway=secondary"));

    avg = uut.average("highway=road", "highway=secondary", score);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, score, 0.001);
    CPPUNIT_ASSERT_EQUAL(avg.toStdString(), std::string("highway=secondary"));

    avg = uut.average("highway=primary", 1, "highway=service", 1, score);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.512, score, 0.001);
    CPPUNIT_ASSERT_EQUAL(avg.toStdString(), std::string("highway=secondary"));

    avg = uut.average("highway=primary", 1, "highway=service", 2, score);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.512, score, 0.001);
    CPPUNIT_ASSERT_EQUAL(avg.toStdString(), std::string("highway=residential"));

    avg = uut.average("highway=primary", 1, "highway=service", 5, score);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.4096, score, 0.001);
    CPPUNIT_ASSERT_EQUAL(avg.toStdString(), std::string("highway=service"));

    avg = uut.average("highway=primary", 5, "highway=service", 1, score);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.4096, score, 0.001);
    CPPUNIT_ASSERT_EQUAL(avg.toStdString(), std::string("highway=primary"));

    avg = uut.average("amenity=bar", 5, "amenity=cafe", 1, score);
    HOOT_STR_EQUALS("amenity=bar", avg.toStdString());

    // when averaging a non-specific tag w/ a tag we know about always return the tag we know.
    avg = uut.average("leisure=badvalue", 5, "leisure=track", 1, score);
    HOOT_STR_EQUALS("leisure=track", avg.toStdString());

    avg = uut.average("leisure=track", 5, "leisure=badvalue", 1, score);
    HOOT_STR_EQUALS("leisure=track", avg.toStdString());

    // when averaging two non-specific tags return the first tag.
    avg = uut.average("leisure=badvalue1", 5, "leisure=badvalue2", 1, score);
    HOOT_STR_EQUALS("leisure=badvalue1", avg.toStdString());
  }

  /**
   * Test calculating the average between two tags with weights.
   */
  void categoryTest()
  {
    OsmSchema uut;

    uut.createTestingGraph();

    Tags tags;
    tags["poi"] = "yes";
    HOOT_STR_EQUALS(1, uut.hasCategory(tags, "poi"));
    HOOT_STR_EQUALS(0, uut.hasCategory(tags, "transportation"));

    tags.clear();
    tags["highway"] = "primary";
    HOOT_STR_EQUALS(0, uut.hasCategory(tags, "poi"));
    HOOT_STR_EQUALS(1, uut.hasCategory(tags, "transportation"));

    vector<TagVertex> tvs = uut.getTagByCategory(OsmSchemaCategory::transportation());
    vector<QString> names;
    for (size_t i = 0; i < tvs.size(); i++)
    {
      names.push_back(tvs[i].name);
    }
    HOOT_STR_EQUALS("[5]{highway=road, highway=primary, highway=secondary, highway=residential, highway=service}",
                    names);
  }

  void commonAncestorTest()
  {
    OsmSchema uut;
    uut.createTestingGraph();

    const TagVertex& v1 = uut.getFirstCommonAncestor("highway=primary", "highway=secondary");
    CPPUNIT_ASSERT_EQUAL(string("road"), v1.value.toStdString());
    const TagVertex& v2 = uut.getFirstCommonAncestor("highway=primary", "highway=primary");
    CPPUNIT_ASSERT_EQUAL(string("primary"), v2.value.toStdString());
    const TagVertex& v3 = uut.getFirstCommonAncestor("highway=road", "highway=primary");
    CPPUNIT_ASSERT_EQUAL(string("road"), v3.value.toStdString());
    const TagVertex& v4 = uut.getFirstCommonAncestor("highway=primary", "highway=road");
    CPPUNIT_ASSERT_EQUAL(string("road"), v4.value.toStdString());
  }

  void dumpAsCsv(OsmSchema& schema, QString tag)
  {
    vector<TagVertex> surfaces = schema.getChildTags(tag);
    QString csvDistance;
    QString csvAverage;

    for (size_t i = 0; i < surfaces.size(); i++)
    {
      csvDistance += ", " + surfaces[i].name;
    }
    csvDistance += "\n";
    csvAverage = csvDistance;

    for (size_t i = 0; i < surfaces.size(); i++)
    {
      csvDistance += surfaces[i].name;
      csvAverage += surfaces[i].name;
      for (size_t j = 0; j < surfaces.size(); j++)
      {
        double d = schema.score(surfaces[i].name, surfaces[j].name);
        csvDistance += QString(", %1").arg(d);
        double best;
        QString avg = schema.average(surfaces[i].name, surfaces[j].name, best);
        csvAverage += QString(", %1").arg(avg);
      }
      csvDistance += "\n";
      csvAverage += "\n";
    }

    QFile csvFp("test-output/" + tag + ".csv");
    csvFp.open(QFile::WriteOnly);
    csvFp.write(csvDistance.toUtf8());
    csvFp.close();

    QFile csvAverageFp("test-output/" + tag + "-average.csv");
    csvAverageFp.open(QFile::WriteOnly);
    csvAverageFp.write(csvAverage.toUtf8());
    csvAverageFp.close();
  }

  void getChildTagsTest()
  {
    OsmSchema& uut = OsmSchema::getInstance();

    vector<TagVertex> gravel = uut.getChildTags("surface=gravel");

    CPPUNIT_ASSERT_EQUAL(2, (int)gravel.size());
  }

  /**
   * Test rudimentary loading of the schema file.
   */
  void loadTest()
  {
    QString hootHome(getenv("HOOT_HOME"));

    OsmSchema uut;
//    JsonSchemaLoader::load(uut, hootHome + "/conf/schema.json");
//    JsonSchemaLoader::load(uut, hootHome + "/conf/schema/schema.json");
    JsonSchemaLoader::load(uut, ConfPath::search("schema.json"));

    QFile fp("tmp/schema.dot");
    fp.open(QFile::WriteOnly);
    fp.write(uut.toGraphvizString().toUtf8());
    fp.close();

//    dumpAsCsv(uut, "surface=unknown");
//    dumpAsCsv(uut, "highway");

    double d;
    d = uut.score("highway=trunk", "highway=motorway");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.8, d, 0.001);
    d = uut.score("highway=path", "highway=motorway");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.168, d, 0.001);
    d = uut.score("highway=path", "highway=road");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.8, d, 0.001);

    QString avg;
    double score;
    avg = uut.average("highway=path", "highway=motorway", score);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.410, score, 0.001);
    CPPUNIT_ASSERT_EQUAL(avg.toStdString(), std::string("highway=tertiary"));

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.7, uut.score("surface=cobblestone", "surface=asphault"), 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, uut.score("surface=cobblestone:flattened",
                                                "surface=cobblestone"), 0.001);
    avg = uut.average("surface=cobblestone:flattened", "surface=asphault", score);
    CPPUNIT_ASSERT_EQUAL(std::string("surface=paved"), avg.toStdString());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(.2, score, 0.001);
    avg = uut.average("surface=cobblestone:flattened", "surface=earth", score);
    CPPUNIT_ASSERT_EQUAL(avg.toStdString(), std::string("surface=unknown"));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(.1, score, 0.001);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, uut.score("parking=surface", "amenity=parking"), 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0, uut.score("parking=surface", "parking=covered"), 0.001);

    // check wildcard mismatchScore for seamark:type.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.1, uut.score("seamark:type=foo", "seamark:type=bar"), 0.001);
    // Check to see if mismatchScore works within amenity types. This doesn't work now, but it'd
    // be good to implement this in the near future.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, uut.score("amenity=auditorium", "amenity=embassy"), 0.001);
  }

  /**
   * Test calculating distance between two tags
   */
  void distanceTest()
  {
    OsmSchema uut;

    uut.createTestingGraph();

    double d = uut.score("highway=primary", "highway=secondary");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.8, d, 0.001);
    d = uut.score("highway=primary", "highway=bad_value");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, d, 0.001);
    d = uut.score("Highway=primary", "highway=Bad_value");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, d, 0.001);
    d = uut.score("highway=secondary", "highway=residential");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.64, d, 0.001);
    d = uut.score("HIGHWAY=Secondary", "hiGHway=ResidenTial");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.64, d, 0.001);
    d = uut.score("highway=primary", "highway=residential");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.512, d, 0.001);
    d = uut.score("highway=primary", "highway=service");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.4096, d, 0.001);
    d = uut.score("highway=primary", "highway=road");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, d, 0.001);
  }

  void getTagTest()
  {
    OsmSchema uut;
    uut.createTestingGraph();

    HOOT_STR_EQUALS("abstract_name", uut.getTagVertex("abstract_name").name);
    HOOT_STR_EQUALS("abstract_name", uut.getTagVertex("abstract_name=foo").name);
    HOOT_STR_EQUALS("name", uut.getTagVertex("name").name);
    HOOT_STR_EQUALS("name", uut.getTagVertex("name=bar").name);
    HOOT_STR_EQUALS("", uut.getTagVertex("poi=foo").name);
    HOOT_STR_EQUALS("poi=yes", uut.getTagVertex("poi=yes").name);
    HOOT_STR_EQUALS("leisure=*", uut.getTagVertex("leisure=foo").name);
  }

  void isAncestorTest()
  {
    OsmSchema uut;
    uut.createTestingGraph();

    CPPUNIT_ASSERT_EQUAL(false, uut.isAncestor("highway=primary", "highway=secondary"));
    CPPUNIT_ASSERT_EQUAL(true, uut.isAncestor("highway=primary", "highway=road"));

    CPPUNIT_ASSERT_EQUAL(true, OsmSchema::getInstance().isAncestor("highway=secondary",
      "highway=road"));
  }

  void isAreaTest()
  {
    OsmSchema uut;
    uut.createTestingGraph();

    Tags t;
    t["area"] = "yes";
    CPPUNIT_ASSERT_EQUAL(true, uut.isArea(t, ElementType::Way));
    CPPUNIT_ASSERT_EQUAL(true, uut.isArea(t, ElementType::Relation));
    CPPUNIT_ASSERT_EQUAL(false, uut.isArea(t, ElementType::Node));

    t.clear();
    t["highway"] = "primary";
    CPPUNIT_ASSERT_EQUAL(false, uut.isArea(t, ElementType::Way));

    t.clear();
    t["building:part"] = "yes";
    CPPUNIT_ASSERT_EQUAL(true, uut.isArea(t, ElementType::Way));
    t["building:part"] = "no";
    CPPUNIT_ASSERT_EQUAL(false, uut.isArea(t, ElementType::Way));
    t["building:part"] = "invalid";
    CPPUNIT_ASSERT_EQUAL(false, uut.isArea(t, ElementType::Way));

    t.clear();
    t["leisure"] = "foo";
    CPPUNIT_ASSERT_EQUAL(true, uut.isArea(t, ElementType::Way));
    CPPUNIT_ASSERT_EQUAL(true, uut.isArea(t, ElementType::Relation));

    t.clear();
    t["leisure"] = "track";
    CPPUNIT_ASSERT_EQUAL(false, uut.isArea(t, ElementType::Way));
    CPPUNIT_ASSERT_EQUAL(false, uut.isArea(t, ElementType::Relation));
  }

  void isMetaDataTest()
  {
    OsmSchema uut;
    //uut.createTestingGraph();
    uut.loadDefault();

    CPPUNIT_ASSERT_EQUAL(false, uut.isMetaData("name", "foo"));
    CPPUNIT_ASSERT_EQUAL(false, uut.isMetaData("foo", "bar"));
    CPPUNIT_ASSERT_EQUAL(false, uut.isMetaData("highway", "service"));
    CPPUNIT_ASSERT_EQUAL(true, uut.isMetaData("security", "U"));
    CPPUNIT_ASSERT_EQUAL(true, uut.isMetaData("security:blah", "dunno"));
    CPPUNIT_ASSERT_EQUAL(true, uut.isMetaData("source", "foo"));
    CPPUNIT_ASSERT_EQUAL(true, uut.isMetaData("source", ""));
    CPPUNIT_ASSERT_EQUAL(true, uut.isMetaData("source:bar", "baz"));
    CPPUNIT_ASSERT_EQUAL(true, uut.isMetaData("created_by", "baz"));
    CPPUNIT_ASSERT_EQUAL(true, uut.isMetaData("source", "foo"));
    CPPUNIT_ASSERT_EQUAL(true, uut.isMetaData("source", ""));

    CPPUNIT_ASSERT_EQUAL(true, OsmSchema::getInstance().isAncestor("highway=secondary",
      "highway=road"));
  }

  void religionTest()
  {
    OsmSchema uut;
    uut.loadDefault();

    double d;
    // These should have a high score. The exact value isn't important.
    d = uut.score("building=mosque", "amenity=place_of_worship");
    CPPUNIT_ASSERT(d >= 0.8);
    // These shouldn't have a high score. The exact score isn't important.
    d = uut.score("building=mosque", "amenity=church");
    CPPUNIT_ASSERT(d <= 0.3);
    // These should have a high score. The exact value isn't important.
    d = uut.score("building=abbey", "amenity=church");
    CPPUNIT_ASSERT(d >= 0.8);
  }



};

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(OsmSchemaTest, "quick");
//CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(OsmSchemaTest, "current");

}
