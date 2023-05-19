#ifndef SPATIALHASH_H_
#define SPATIALHASH_H_

#include <vector>
#include <unordered_map>
#include <ngl/Vec3.h>

class SpatialHash 
{
public:
  SpatialHash(float cellSize) noexcept : m_cellSize(cellSize) {}

  void insert(const ngl::Vec3& point) noexcept;
  void debug() const noexcept;
  void clear() noexcept;
  size_t size() const noexcept;

  std::vector<ngl::Vec3> query(const ngl::Vec3& point, float radius) const noexcept ;
private:
  float m_cellSize = 1.0f;
  std::unordered_map<int, std::vector<ngl::Vec3>> m_hashTable;

  int hash(const ngl::Vec3& point) const  noexcept;

  int hashCoord(float coord) const noexcept ;

  int hashIndex(int x, int y, int z) const noexcept;
};

#endif