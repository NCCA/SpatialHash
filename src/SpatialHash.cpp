#include "SpatialHash.h"
#include <iostream>
#include <ngl/NGLStream.h>
#include <ngl/NGLHash.h>


  void SpatialHash::insert(const ngl::Vec3& point) 
  {
    int index = hash(point);
    m_hashTable[index].push_back(point);
    
  }

  void SpatialHash::clear()
  {
    m_hashTable.clear();
  }

  std::vector<ngl::Vec3> SpatialHash::query(const ngl::Vec3& point, float radius) const 
  {
    std::vector<ngl::Vec3> result;
    int minIndexX = hashCoord(point.m_x - radius);
    int maxIndexX = hashCoord(point.m_x + radius);
    int minIndexY = hashCoord(point.m_y - radius);
    int maxIndexY = hashCoord(point.m_y + radius);
    int minIndexZ = hashCoord(point.m_z - radius);
    int maxIndexZ = hashCoord(point.m_z + radius);
    for (int i = minIndexX; i <= maxIndexX; i++) 
    {
      for (int j = minIndexY; j <= maxIndexY; j++) 
      {
        for (int k = minIndexZ; k <= maxIndexZ; k++) 
        {
          int index = hashIndex(i, j, k);
          auto it = m_hashTable.find(index);
          if (it != m_hashTable.end()) 
          {
            for (const auto& p : it->second) 
            {
              if ((p - point).lengthSquared() <= radius * radius) 
              {
                result.push_back(p);
              }
            }
          }
        }
      }
    }
    return result;
  }
  int SpatialHash::hash(const ngl::Vec3& point) const 
  {
    int x = hashCoord(point.m_x);
    int y = hashCoord(point.m_y);
    int z = hashCoord(point.m_z);
    return hashIndex(x, y, z);
  }

  int SpatialHash::hashCoord(float coord) const 
  {
    return static_cast<int>(std::floor(coord / m_cellSize));
  }

  int SpatialHash::hashIndex(int x, int y, int z) const 
  {
    return x + y * 15485863 + z * 32452843;
  }

  void SpatialHash::debug() const 
  {
   std::cout<<"************************************\n";
    for(auto h : m_hashTable)
    {
      std::cout<<"hash "<<h.first<<'\n';
      for(auto p : h.second)
      {
        std::cout<<"point "<<p<<'\n';
      }
    }
    std::cout<<"************************************\n";
  }

  size_t SpatialHash::size() const 
  {
    return m_hashTable.size();
  }