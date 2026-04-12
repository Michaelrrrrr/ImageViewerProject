#include "ThreeDobj.h"
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <vector>
#define M_PI 3.14159265358979323846

bool ThreeDobj::saveVtk(const QString& filename) {
    std::ofstream file(filename.toStdString());
    if (!file.is_open()) return false;
    file << "# vtk DataFile Version 3.0\n";
    file << "3D Object Model\nASCII\nDATASET POLYDATA\n";

    file << "POINTS " << vertexes.size() << " float\n";
    for (const auto& v : vertexes) {
        file << v.x << " " << v.y << " " << v.z << "\n";
    }

    file << "POLYGONS " << faces.size() << " " << faces.size() * 4 << "\n";
    for (const auto& f : faces) {
        file << "3 " << f.idx[0] << " " << f.idx[1] << " " << f.idx[2] << "\n";
    }
    file.close();
    return true;
}

bool ThreeDobj::loadVtk(const QString& filename) {
    std::ifstream file(filename.toStdString());
    if (!file.is_open()) return false;

    clear();
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);
        std::string tag;
        ss >> tag;

        if (tag == "POINTS") {
            int count;
            ss >> count;
            for (int i = 0; i < count; i++) {
                float x, y, z;
                file >> x >> y >> z;
                vertexes.push_back({ x, y, z });
            }
        }
        else if (tag == "POLYGONS") {
            int numFaces, total;
            ss >> numFaces >> total;
            for (int i = 0; i < numFaces; i++) {
                int n, i1, i2, i3;
                file >> n >> i1 >> i2 >> i3;
                if (n == 3) {
                    faces.push_back({ {i1, i2, i3} });
                }
            }
        }
    }

    file.close();
    return !vertexes.empty();
}

void ThreeDobj::createCube(double size) {
    clear();

    double h = size / 2.0;

    // верхня
    vertexes.push_back({ (float)-h, (float)-h, (float)-h }); // 0
    vertexes.push_back({ (float)h,  (float)-h, (float)-h }); // 1
    vertexes.push_back({ (float)h,  (float)h,  (float)-h }); // 2
    vertexes.push_back({ (float)-h, (float)h,  (float)-h }); // 3
    // нижня
    vertexes.push_back({ (float)-h, (float)-h, (float)h });  // 4
    vertexes.push_back({ (float)h,  (float)-h, (float)h });  // 5
    vertexes.push_back({ (float)h,  (float)h, (float)h });  // 6
    vertexes.push_back({ (float)-h, (float)h, (float)h });  // 7

    faces.push_back({ {0, 1, 2} }); faces.push_back({ {0, 2, 3} }); // Нижня
    faces.push_back({ {4, 6, 5} }); faces.push_back({ {4, 7, 6} }); // Верхня
    faces.push_back({ {0, 4, 1} }); faces.push_back({ {1, 4, 5} }); // Передня
    faces.push_back({ {2, 6, 3} }); faces.push_back({ {3, 6, 7} }); // Задня
    faces.push_back({ {0, 3, 4} }); faces.push_back({ {4, 3, 7} }); // Ліва
    faces.push_back({ {1, 5, 2} }); faces.push_back({ {2, 5, 6} }); // Права
}

void ThreeDobj::createUVSphere(int parallels, int meridians, double radius) {
    clear();

    for (int i = 0; i <= parallels; i++) {
        double theta = M_PI * i / parallels;
        double sinTheta = std::sin(theta);
        double cosTheta = std::cos(theta);

        for (int j = 0; j <= meridians; j++) {
            double phi = 2.0 * M_PI * j / meridians;

            float x = (float)(radius * sinTheta * std::cos(phi));
            float y = (float)(radius * cosTheta);
            float z = (float)(radius * sinTheta * std::sin(phi));

            vertexes.push_back({ x, y, z });
        }
    }

    // з'єднуємо сусідні точки, розраховуємо індекси 4-х точок одного прямокутника сітки, ділимо прямокутник на два трикутники
    for (int i = 0; i < parallels; i++) {
        for (int j = 0; j < meridians; j++) {
            
            int p1 = i * (meridians + 1) + j;
            int p2 = (i + 1) * (meridians + 1) + j;
            int p3 = p1 + 1;
            int p4 = p2 + 1;

            faces.push_back({ {p1, p2, p3} });
            faces.push_back({ {p2, p4, p3} });
        }
    }
}
