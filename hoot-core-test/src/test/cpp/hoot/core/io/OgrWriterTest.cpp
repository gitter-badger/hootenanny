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
 * @copyright Copyright (C) 2013, 2014 DigitalGlobe (http://www.digitalglobe.com/)
 */

// CPP Unit
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>

// Hoot
#include <hoot/core/io/OgrWriter.h>
#include <hoot/core/io/OsmMapWriterFactory.h>
#include <hoot/core/util/FileUtils.h>

// Qt
#include <QDir>

#include "../TestUtils.h"

namespace hoot
{

class OgrWriterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(OgrWriterTest);
  CPPUNIT_TEST(runGdbTest);
  CPPUNIT_TEST(runShpTest);
  CPPUNIT_TEST_SUITE_END();

public:

  boost::shared_ptr<OsmMap> _map;

  boost::shared_ptr<Node> createNode(double x, double y)
  {
    boost::shared_ptr<Node> n(new Node(Status::Unknown1, _map->createNextNodeId(), x, y, 10.0));
    _map->addNode(n);
    return n;
  }

  boost::shared_ptr<OsmMap> createTestMap()
  {
    boost::shared_ptr<OsmMap> map(new OsmMap());
    _map = map;

    boost::shared_ptr<Node> n1 = createNode(0.0, 0.0);
    n1->setTag("building", "yes");
    n1->setTag("name", "n1");

    boost::shared_ptr<Way> w1(new Way(Status::Unknown1, map->createNextWayId(), 13.0));
    w1->setTag("area", "yes");
    w1->setTag("building", "yes");
    w1->setTag("name", "w1");
    w1->addNode(createNode(0.1, 0.0)->getId());
    w1->addNode(createNode(0.2, 0.0)->getId());
    w1->addNode(createNode(0.2, 0.1)->getId());
    w1->addNode(w1->getNodeId(0));
    map->addWay(w1);

    boost::shared_ptr<Way> w2(new Way(Status::Unknown1, map->createNextWayId(), 13.0));
    w2->setTag("highway", "track");
    w2->setTag("name", "w2");
    w2->addNode(createNode(0.3, 0.0)->getId());
    w2->addNode(createNode(0.3, 0.1)->getId());
    map->addWay(w2);

    boost::shared_ptr<Way> w3(new Way(Status::Unknown1, map->createNextWayId(), 13.0));
    w3->setTag("highway", "road");
    w3->setTag("name", "w3");
    w3->addNode(createNode(0.4, 0.0)->getId());
    w3->addNode(createNode(0.4, 0.1)->getId());
    map->addWay(w3);

    boost::shared_ptr<Way> w4(new Way(Status::Unknown1, map->createNextWayId(), 13.0));
    w4->addNode(createNode(0.5, 0.0)->getId());
    w4->addNode(createNode(0.7, 0.0)->getId());
    w4->addNode(createNode(0.6, 0.1)->getId());
    w4->addNode(w4->getNodeId(0));
    map->addWay(w4);

    boost::shared_ptr<Way> w5(new Way(Status::Unknown1, map->createNextWayId(), 13.0));
    w5->addNode(createNode(0.55, 0.01)->getId());
    w5->addNode(createNode(0.65, 0.01)->getId());
    w5->addNode(createNode(0.6, 0.05)->getId());
    w5->addNode(w5->getNodeId(0));
    map->addWay(w5);

    boost::shared_ptr<Relation> r1(new Relation(Status::Unknown1, 1, 15.0, "multipolygon"));
    r1->setTag("building", "yes");
    r1->setTag("name", "r1");
    r1->addElement("outer", w4->getElementId());
    r1->addElement("inner", w5->getElementId());
    map->addRelation(r1);

    return map;
  }

  void runShpTest()
  {
    OgrWriter uut;
    uut.setScriptPath("test-files/io/SampleTranslation.js");
    FileUtils::removeDir("test-output/io/OgrWriterTest");
    uut.open("test-output/io/OgrWriterTest.shp");
    uut.write(createTestMap());
    QStringList nameFilter;
    nameFilter << "*.shp";

    // make sure it created the shapefiles, we aren't actually testing for correct output.
    HOOT_STR_EQUALS("[4]{AAL015.shp, LAP010.shp, LAP030.shp, PAL015.shp}",
                    QDir("test-output/io/OgrWriterTest").entryList(nameFilter));
  }

  void runGdbTest()
  {
    OgrWriter uut;
    uut.setScriptPath("test-files/io/SampleTranslation.js");
    FileUtils::removeDir("test-output/io/OgrWriterTest.gdb");
    uut.open("test-output/io/OgrWriterTest.gdb");
    uut.write(createTestMap());

    QDir().mkpath("tmp");
    OsmMapWriterFactory::write(createTestMap(), "tmp/TestMap.osm");

    // make sure it created a bunch of files. We aren't testing for correct output.
    CPPUNIT_ASSERT(QDir("test-output/io/OgrWriterTest.gdb").entryList().size() > 10);
  }
};

//CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(OgrWriterTest, "current");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(OgrWriterTest, "quick");

}
