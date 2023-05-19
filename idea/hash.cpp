#include <unordered_map>
#include <vector>
#include <ngl/Vec3.h>
#include <ngl/NGLHash.h> // added
#include <iostream>
#include <ngl/NGLStream.h>
class SpatialHash
{
public:
    SpatialHash(float cellSize) : m_cellSize(cellSize) {}

    void insert(const ngl::Vec3& point) 
    {
        std::vector<ngl::Vec3> cellPoints = getCellPoints(point);
        for (const auto& cellPoint : cellPoints) 
        {
            m_cells[cellPoint].push_back(point);
        }
    }

    std::vector<ngl::Vec3> query(const ngl::Vec3& point, float radius) const 
    {
        std::vector<ngl::Vec3> result;
        // not radius was a single ctor here
        std::vector<ngl::Vec3> cellPoints = getCellPoints(point - ngl::Vec3(radius,radius,radius));
        for (const auto& cellPoint : cellPoints) 
        {
            auto cellIter = m_cells.find(cellPoint);
            if (cellIter != m_cells.end()) 
            {
                for (const auto& cellPoint : cellIter->second) 
                {
                    if ((cellPoint - point).lengthSquared() <= radius * radius) 
                    {
                        result.push_back(cellPoint);
                    }
                }
            }
        }
        return result;
    }

private:
    std::vector<ngl::Vec3> getCellPoints(const ngl::Vec3& point) const
     {
        int x = static_cast<int>(std::floor(point.m_x / m_cellSize));
        int y = static_cast<int>(std::floor(point.m_y / m_cellSize));
        int z = static_cast<int>(std::floor(point.m_z / m_cellSize));
        std::vector<ngl::Vec3> cellPoints;
        cellPoints.reserve(8);
        for (int i = 0; i < 2; ++i) 
        {
            for (int j = 0; j < 2; ++j) 
            {
                for (int k = 0; k < 2; ++k) 
                {
                    cellPoints.emplace_back((x + i) * m_cellSize, (y + j) * m_cellSize, (z + k) * m_cellSize);
                }
            }
        }
        return cellPoints;
    }

    float m_cellSize;
    std::unordered_map<ngl::Vec3, std::vector<ngl::Vec3>> m_cells;
};


int main()
{
    SpatialHash hash(1.0f);
    hash.insert(ngl::Vec3(0.0f, 0.0f, 0.0f));
    hash.insert(ngl::Vec3(10.0f, 0.0f, 0.0f));
    hash.insert(ngl::Vec3(0.0f, 1.0f, 0.0f));
    hash.insert(ngl::Vec3(0.0f, 0.0f, 1.0f));
    hash.insert(ngl::Vec3(1.0f, 1.0f, 0.0f));
    hash.insert(ngl::Vec3(1.0f, 0.0f, 1.0f));
    hash.insert(ngl::Vec3(0.0f, 1.0f, 1.0f));
    hash.insert(ngl::Vec3(1.0f, 1.0f, 1.0f));
    auto result = hash.query(ngl::Vec3(0.0f, 0.0f, 0.0f), 4.0f);
    for(auto r : result)
    {
        std::cout << r << '\n';
    }
    return 0;
}