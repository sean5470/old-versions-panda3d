// Filename: movingPartMatrix.cxx
// Created by:  drose (23Feb99)
// 
////////////////////////////////////////////////////////////////////

#include "movingPartMatrix.h"

#include <compose_matrix.h>
#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma implementation
#endif

TypeHandle MovingPartMatrix::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: MovingPartMatrix::get_blend_value
//       Access: Public
//  Description: Attempts to blend the various matrix values
//               indicated, and sets the _value member to the
//               resulting matrix.
////////////////////////////////////////////////////////////////////
void MovingPartMatrix::
get_blend_value(const PartBundle *root) {
  const PartBundle::ChannelBlend &blend = root->get_blend_map();

  if (blend.empty()) {
    // No channel is bound; supply the default value.
    _value = _initial_value;

  } else if (blend.size() == 1) {
    // A single value, the normal case.
    AnimControl *control = (*blend.begin()).first;

    int channel_index = control->get_channel_index();
    nassertv(channel_index >= 0 && channel_index < (int)_channels.size());
    ChannelType *channel = DCAST(ChannelType, _channels[channel_index]);
    nassertv(channel != NULL);
    
    channel->get_value(control->get_frame(), _value);

  } else {
    // A blend of two or more values.

    if (root->get_blend_type() == PartBundle::BT_linear) {
      // An ordinary, linear blend.
      _value = 0.0;
      float net = 0.0;
      
      PartBundle::ChannelBlend::const_iterator cbi;
      for (cbi = blend.begin(); cbi != blend.end(); ++cbi) {
	AnimControl *control = (*cbi).first;
	float effect = (*cbi).second;
	nassertv(effect != 0.0);
	
	int channel_index = control->get_channel_index();
	nassertv(channel_index >= 0 && channel_index < (int)_channels.size());
	ChannelType *channel = DCAST(ChannelType, _channels[channel_index]);
	nassertv(channel != NULL);
	
	ValueType v;
	channel->get_value(control->get_frame(), v);
	
	_value += v * effect;
	net += effect;
      }
      
      nassertv(net != 0.0);
      _value /= net;

    } else if (root->get_blend_type() == PartBundle::BT_normalized_linear) {
      // A normalized linear blend.  This means we do a linear blend
      // without scales, normalize the scale components of the
      // resulting matrix to eliminate artificially-introduced scales,
      // and then reapply the scales.

      _value = 0.0;
      LVector3f scale(0.0, 0.0, 0.0);
      float net = 0.0;
      
      PartBundle::ChannelBlend::const_iterator cbi;
      for (cbi = blend.begin(); cbi != blend.end(); ++cbi) {
	AnimControl *control = (*cbi).first;
	float effect = (*cbi).second;
	nassertv(effect != 0.0);
	
	int channel_index = control->get_channel_index();
	nassertv(channel_index >= 0 && channel_index < (int)_channels.size());
	ChannelType *channel = DCAST(ChannelType, _channels[channel_index]);
	nassertv(channel != NULL);
	
	ValueType v;
	channel->get_value_no_scale(control->get_frame(), v);
	LVector3f s;
	channel->get_scale(control->get_frame(), &s[0]);
	
	_value += v * effect;
	scale += s * effect;
	net += effect;
      }
      
      nassertv(net != 0.0);
      _value /= net;
      scale /= net;

      // Now rebuild the matrix with the correct scale values.

      LVector3f false_scale, hpr, translate;
      decompose_matrix(_value, false_scale, hpr, translate);
      compose_matrix(_value, scale, hpr, translate);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MovingPartMatrix::make_MovingPartMatrix
//       Access: Protected
//  Description: Factory method to generate a MovingPartMatrix object
////////////////////////////////////////////////////////////////////
TypedWriteable* MovingPartMatrix::
make_MovingPartMatrix(const FactoryParams &params)
{
  MovingPartMatrix *me = new MovingPartMatrix;
  BamReader *manager;
  Datagram packet;

  parse_params(params, manager, packet);
  DatagramIterator scan(packet);

  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: MovingPartMatrix::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a MovingPartMatrix object
////////////////////////////////////////////////////////////////////
void MovingPartMatrix::
register_with_read_factory(void)
{
  BamReader::get_factory()->register_factory(get_class_type(), make_MovingPartMatrix);
}
