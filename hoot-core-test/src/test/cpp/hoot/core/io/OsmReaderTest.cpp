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
#include <hoot/core/OsmMap.h>
#include <hoot/core/io/OsmReader.h>
#include <hoot/core/io/OsmWriter.h>
using namespace hoot;


// Boost
using namespace boost;

// Qt
#include <QDebug>

#include "../TestUtils.h"

class OsmReaderTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(OsmReaderTest);
    CPPUNIT_TEST(runTest);
    CPPUNIT_TEST(runUseIdTest);
    CPPUNIT_TEST(runUseStatusTest);
    CPPUNIT_TEST_SUITE_END();

public:

    void runTest()
    {
        OsmReader uut;

        boost::shared_ptr<OsmMap> map(new OsmMap());
        uut.read("test-files/ToyTestA.osm", map);

        CPPUNIT_ASSERT_EQUAL(36, map->getNodeMap().size());
        CPPUNIT_ASSERT_EQUAL(4, (int)map->getWays().size());

        OsmWriter writer;
        writer.write(map, "output.osm");
    }

    void runUseIdTest()
    {
        OsmReader uut;

        OsmMap::resetCounters();

        boost::shared_ptr<OsmMap> map(new OsmMap());
        uut.setUseDataSourceIds(true);
        uut.read("test-files/ToyTestA.osm", map);

        CPPUNIT_ASSERT_EQUAL(36, map->getNodeMap().size());
        CPPUNIT_ASSERT_EQUAL(4, (int)map->getWays().size());

        long min = 1e9;
        long max = -1e9;
        for (OsmMap::NodeMap::const_iterator it = map->getNodeMap().constBegin();
             it != map->getNodeMap().constEnd(); ++it)
        {
          const boost::shared_ptr<const Node>& n = it.value();
          min = std::min(min, n->getId());
          max = std::max(max, n->getId());
        }

        CPPUNIT_ASSERT_EQUAL(-1669793l, min);
        CPPUNIT_ASSERT_EQUAL(-1669723l, max);

        CPPUNIT_ASSERT(map->containsWay(-1669801));
        CPPUNIT_ASSERT(map->containsWay(-1669799));
        CPPUNIT_ASSERT(map->containsWay(-1669797));
        CPPUNIT_ASSERT(map->containsWay(-1669795));

        OsmWriter writer;
        writer.write(map, "output.osm");
    }

    void runUseStatusTest()
    {
        OsmReader uut;

        OsmMap::resetCounters();

        boost::shared_ptr<OsmMap> map(new OsmMap());
        uut.setUseDataSourceIds(true);
        uut.setUseStatusFromFile(true);
        uut.setDefaultStatus(Status::Invalid);
        uut.read("test-files/io/OsmReaderUseStatusTest.osm", map);

        CPPUNIT_ASSERT_EQUAL(104, map->getNodeMap().size());
        CPPUNIT_ASSERT_EQUAL(17, (int)map->getWays().size());

        CPPUNIT_ASSERT_EQUAL(Status::Unknown1, map->getWay(-12)->getStatus().getEnum());
        CPPUNIT_ASSERT_EQUAL(Status::Conflated, map->getWay(-13)->getStatus().getEnum());
        CPPUNIT_ASSERT_EQUAL(Status::Unknown2, map->getWay(-51)->getStatus().getEnum());

        OsmWriter writer;
        writer.write(map, "output.osm");
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsmReaderTest);

