/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef CONFLATEREDUCER_H
#define CONFLATEREDUCER_H

// Hoot
#include <hoot/core/OsmMap.h>
#include <hoot/core/conflate/NodeReplacements.h>
#include <hoot/hadoop/HadoopIdGenerator.h>
#include <hoot/hadoop/MapStats.h>
#include <hoot/hadoop/PbfRecordWriter.h>

// Pretty Pipes
#include <pp/mapreduce/Reducer.h>

namespace hoot
{

class ConflateReducer : public pp::Reducer
{
public:
  static string className() { return "hoot::ConflateReducer"; }

  ConflateReducer();

  virtual void close();

  virtual void reduce(HadoopPipes::ReduceContext& context);

private:

  vector<Envelope> _envelopes;

  bool _initialized;

  /// Maximum way size in degrees.
  double _maxWaySize;
  double _buffer;
  NodeReplacements _nr;

  MapStats _stats;
  shared_ptr<HadoopIdGenerator> _idGen;
  int _partition;
  std::string _workDir;
  PbfRecordWriter* _writer;

  void _conflate(int key, HadoopPipes::ReduceContext& context);

  void _emitMap(shared_ptr<OsmMap> map);

  const Envelope& _getContainingEnvelope(const shared_ptr<OsmMap>& map);

  void _init(HadoopPipes::ReduceContext& context);

  shared_ptr<OsmMap> _readMap(const string& value);

  void _removeReplacedNodes(shared_ptr<OsmMap>& map);

  void _splitWay(shared_ptr<Way> w);

  void _validate(const shared_ptr<OsmMap>& map);

  void _writeNodes(HadoopPipes::ReduceContext& context);
  void _writeWay(HadoopPipes::ReduceContext& context);
};

}

#endif // CONFLATEREDUCER_H
