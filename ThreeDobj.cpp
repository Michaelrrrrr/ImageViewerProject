#include "ThreeDobj.h"
#include <fstream>
#include <sstream>
#include <string>

bool ThreeDobj::saveVtk(const QString& filename) const {
    std::ofstream out(filename.toStdString());
    if (!out) return false;
    out << "# vtk DataFile Version 3.0\n";
    out << "vtk output\nASCII\nDATASET POLYDATA\n";
    out << "POINTS " << vertexes.size() << " float\n";
    for (const QVector3D& v : vertexes)
        out << v.x() << ' ' << v.y() << ' ' << v.z() << '\n';
    out << "POLYGONS " << faces.size() << ' ' << faces.size() * 4 << '\n';
    for (const QVector<int>& face : faces)
        out << face.size() << ' ' << face[0] << ' ' << face[1] << ' ' << face[2] << '\n';
    return true;
}

bool ThreeDobj::loadVtk(const QString& filename) {
    std::ifsream in(filename.toStdString());
    if (!in) return false;
    clear();
    std::string line;
    bool readingPoints = false, readingPolys = false;
    int pointsLeft = 0, polysLeft = 0;
    while (std::getline())
}
