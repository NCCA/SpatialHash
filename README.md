# Spatial Hash

This demo is a simple implementation of a spatial hash. It is used to speed up collision detection in games. 

Most of the hash code was written by Github Co-Pilot with some changes by me to allow it to work with ngl. 

The inital prompt was 

write a c++ class for spatial hashing a 3d point

> Here's an example of a C++ class for spatial hashing a 3D point:

```cpp
#include <vector>
#include <unordered_map>
#include <ngl/Vec3.h>

class SpatialHash {
public:
  SpatialHash(float cellSize) : m_cellSize(cellSize) {}

  void insert(const ngl::Vec3& point) {
    int index = hash(point);
    m_hashTable[index].push_back(point);
  }

  std::vector<ngl::Vec3> query(const ngl::Vec3& point, float radius) const {
    std::vector<ngl::Vec3> result;
    int minIndexX = hashCoord(point.x - radius);
    int maxIndexX = hashCoord(point.x + radius);
    int minIndexY = hashCoord(point.y - radius);
    int maxIndexY = hashCoord(point.y + radius);
    int minIndexZ = hashCoord(point.z - radius);
    int maxIndexZ = hashCoord(point.z + radius);
    for (int i = minIndexX; i <= maxIndexX; i++) {
      for (int j = minIndexY; j <= maxIndexY; j++) {
        for (int k = minIndexZ; k <= maxIndexZ; k++) {
          int index = hashIndex(i, j, k);
          auto it = m_hashTable.find(index);
          if (it != m_hashTable.end()) {
            for (const auto& p : it->second) {
              if ((p - point).lengthSquared() <= radius * radius) {
                result.push_back(p);
              }
            }
          }
        }
      }
    }
    return result;
  }

private:
  float m_cellSize;
  std::unordered_map<int, std::vector<ngl::Vec3>> m_hashTable;

  int hash(const ngl::Vec3& point) const {
    int x = hashCoord(point.x);
    int y = hashCoord(point.y);
    int z = hashCoord(point.z);
    return hashIndex(x, y, z);
  }

  int hashCoord(float coord) const {
    return static_cast<int>(std::floor(coord / m_cellSize));
  }

  int hashIndex(int x, int y, int z) const {
    return x + y * 15485863 + z * 32452843;
  }
};
```
This class defines a spatial hash for 3D points using an unordered map to store the points in cells. The constructor takes a float argument cellSize that specifies the size of each cell in the hash. The insert method inserts a point into the hash by computing the cells that the point belongs to and adding the point to each cell. The query method returns a vector of points within a given radius of a query point by computing the cells that intersect the query sphere and checking each point in those cells for proximity to the query point.

Note that this implementation assumes that the points are represented as ngl::Vec3 objects. You may need to modify the code to work with a different point representation.


I then asked it to write tests (which still fail)
```cpp
#include "gtest/gtest.h"
#include "SpatialHash.h"

TEST(SpatialHashTest, InsertAndQuery) {
  SpatialHash hash(1.0f);
  hash.insert(ngl::Vec3(0.0f, 0.0f, 0.0f));
  hash.insert(ngl::Vec3(1.0f, 0.0f, 0.0f));
  hash.insert(ngl::Vec3(0.0f, 1.0f, 0.0f));
  hash.insert(ngl::Vec3(0.0f, 0.0f, 1.0f));
  hash.insert(ngl::Vec3(1.0f, 1.0f, 1.0f));
  auto result = hash.query(ngl::Vec3(0.5f, 0.5f, 0.5f), 0.6f);
  ASSERT_EQ(result.size(), 4);
  EXPECT_EQ(result[0], ngl::Vec3(0.0f, 0.0f, 0.0f));
  EXPECT_EQ(result[1], ngl::Vec3(1.0f, 0.0f, 0.0f));
  EXPECT_EQ(result[2], ngl::Vec3(0.0f, 1.0f, 0.0f));
  EXPECT_EQ(result[3], ngl::Vec3(0.0f, 0.0f, 1.0f));
}

TEST(SpatialHashTest, QueryEmpty) {
  SpatialHash hash(1.0f);
  auto result = hash.query(ngl::Vec3(0.0f, 0.0f, 0.0f), 1.0f);
  ASSERT_EQ(result.size(), 0);
}

```


# Analysis

Whilst it thinks it's good to use ngl::Vec3 (this is what I use and it didn't know that) it still doesn't know the attributes are .m_x .m_y .m_z so I had to modify that.

I needed to add a clear function for my use case in the demo.

I re asked it with a slightly different prompt and got this code

```cpp
#include <unordered_map>
#include <vector>
#include <ngl/Vec3.h>

class SpatialHash {
public:
    SpatialHash(float cellSize) : m_cellSize(cellSize) {}

    void insert(const ngl::Vec3& point) {
        std::vector<ngl::Vec3> cellPoints = getCellPoints(point);
        for (const auto& cellPoint : cellPoints) {
            m_cells[cellPoint].push_back(point);
        }
    }

    std::vector<ngl::Vec3> query(const ngl::Vec3& point, float radius) const {
        std::vector<ngl::Vec3> result;
        std::vector<ngl::Vec3> cellPoints = getCellPoints(point - ngl::Vec3(radius));
        for (const auto& cellPoint : cellPoints) {
            auto cellIter = m_cells.find(cellPoint);
            if (cellIter != m_cells.end()) {
                for (const auto& cellPoint : cellIter->second) {
                    if ((cellPoint - point).lengthSquared() <= radius * radius) {
                        result.push_back(cellPoint);
                    }
                }
            }
        }
        return result;
    }

private:
    std::vector<ngl::Vec3> getCellPoints(const ngl::Vec3& point) const {
        int x = static_cast<int>(std::floor(point.x / m_cellSize));
        int y = static_cast<int>(std::floor(point.y / m_cellSize));
        int z = static_cast<int>(std::floor(point.z / m_cellSize));
        std::vector<ngl::Vec3> cellPoints;
        cellPoints.reserve(8);
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 2; ++j) {
                for (int k = 0; k < 2; ++k) {
                    cellPoints.emplace_back((x + i) * m_cellSize, (y + j) * m_cellSize, (z + k) * m_cellSize);
                }
            }
        }
        return cellPoints;
    }

    float m_cellSize;
    std::unordered_map<ngl::Vec3, std::vector<ngl::Vec3>> m_cells;
};
```

Not tried this yet I will compare and do benchmarks when I get time.