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

// Hoot
#include <hoot/core/OsmMap.h>
#include <hoot/core/io/OsmReader.h>
#include <hoot/core/io/OsmWriter.h>
#include <hoot/core/ops/BuildingOutlineUpdateOp.h>
#include <hoot/core/util/Log.h>
using namespace hoot;

// Boost
using namespace boost;

// CPP Unit
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>

// hoot
#include <hoot/core/MapReprojector.h>

// Qt
#include <QDebug>
#include <QDir>

#include "../TestUtils.h"

namespace hoot
{

class BuildingOutlineUpdateOpTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BuildingOutlineUpdateOpTest);
  CPPUNIT_TEST(runSelfIntersectingRelationTest);
  CPPUNIT_TEST_SUITE_END();

public:

  void runSelfIntersectingRelationTest()
  {
    DisableLog dl;

    OsmReader reader;

    shared_ptr<OsmMap> map(new OsmMap());
    OsmMap::resetCounters();
    reader.setDefaultStatus(Status::Unknown1);
    reader.read("test-files/ops/BuildingOutlineUpdateOp/SelfIntersectingRelationsIn.osm", map);

    BuildingOutlineUpdateOp uut;
    uut.apply(map);

    MapReprojector::reprojectToWgs84(map);

    // This output includes two reviews instead of the expected 1 review. See ticket #7043 for
    // an idea to clean this up.
    QDir().mkpath("test-output/ops/BuildingOutlineUpdateOp/");
    OsmWriter writer;
    writer.write(map, "test-output/ops/BuildingOutlineUpdateOp/SelfIntersectingRelationsOut.osm");
    HOOT_FILE_EQUALS("test-files/ops/BuildingOutlineUpdateOp/SelfIntersectingRelationsOut.osm",
                     "test-output/ops/BuildingOutlineUpdateOp/SelfIntersectingRelationsOut.osm");
  }

};

}

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(BuildingOutlineUpdateOpTest, "quick");
//CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(BuildingOutlineUpdateOpTest, "current");


