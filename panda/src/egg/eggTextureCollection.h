// Filename: eggTextureCollection.h
// Created by:  drose (15Feb00)
//
////////////////////////////////////////////////////////////////////

#ifndef EGGTEXTURECOLLECTION_H
#define EGGTEXTURECOLLECTION_H

#include <pandabase.h>

#include "eggTexture.h"
#include "eggGroupNode.h"

#include <string>
#include <map>

////////////////////////////////////////////////////////////////////
// 	 Class : EggTextureCollection
// Description : This is a collection of textures by TRef name.  It
//               can extract the textures from an egg file and sort
//               them all together; it can also manage the creation of
//               unique textures and the assignment of unique TRef
//               names.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggTextureCollection {

  // This is a bit of private interface stuff that must be here as a
  // forward reference.  This allows us to define the
  // EggTextureCollection as an STL container.

private:
  typedef map<PT(EggTexture), int> Textures;
  typedef vector< PT(EggTexture) > OrderedTextures;

public:
  typedef OrderedTextures::const_iterator iterator;
  typedef iterator const_iterator;
  typedef OrderedTextures::size_type size_type;

  typedef map<PT(EggTexture),  PT(EggTexture) > TextureReplacement;

  // Here begins the actual public interface to EggTextureCollection.

public:
  EggTextureCollection();
  EggTextureCollection(const EggTextureCollection &copy);
  EggTextureCollection &operator = (const EggTextureCollection &copy);

  void clear();

  int extract_textures(EggGroupNode *node);
  int insert_textures(EggGroupNode *node);
  int insert_textures(EggGroupNode *node, EggGroupNode::iterator position);

  int find_used_textures(EggNode *node);
  void remove_unused_textures(EggNode *node);

  int collapse_equivalent_textures(int eq, EggGroupNode *node);
  int collapse_equivalent_textures(int eq, TextureReplacement &removed);
  static void replace_textures(EggGroupNode *node,
			       const TextureReplacement &replace);

  void uniquify_trefs();
  void sort_by_tref();

  // Can be used to traverse all the textures in the collection, in
  // order as last sorted.
  INLINE iterator begin() const;
  INLINE iterator end() const;
  INLINE bool empty() const;
  INLINE size_type size() const;

  bool add_texture(EggTexture *texture);
  bool remove_texture(EggTexture *texture);

  // create_unique_texture() creates a new texture if there is not
  // already one equivalent (according to eq, see
  // EggTexture::is_equivalent_to()) to the indicated texture, or
  // returns the existing one if there is.
  EggTexture *create_unique_texture(const EggTexture &copy, int eq);

  // Find a texture with a particular TRef name.
  EggTexture *find_tref(const string &tref_name) const;

  // Find a texture with a particular filename.
  EggTexture *find_filename(const Filename &filename) const;

private:
  Textures _textures;
  OrderedTextures _ordered_textures;
};

#include "eggTextureCollection.I"

#endif
