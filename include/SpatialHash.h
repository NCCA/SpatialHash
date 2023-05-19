#ifndef SPATIALHASH_H_
#define SPATIALHASH_H_

#include <vector>
#include <unordered_map>
#include <ngl/Vec3.h>

class SpatialHash 
{
public:
  SpatialHash(float cellSize) : m_cellSize(cellSize) {}

  void insert(const ngl::Vec3& point);
  void debug() const;
  void clear();
  size_t size() const;

  std::vector<ngl::Vec3> query(const ngl::Vec3& point, float radius) const ;
private:
  float m_cellSize = 1.0f;
  std::unordered_map<int, std::vector<ngl::Vec3>> m_hashTable;

  int hash(const ngl::Vec3& point) const ;

  int hashCoord(float coord) const ;

  int hashIndex(int x, int y, int z) const ;
};

#endif