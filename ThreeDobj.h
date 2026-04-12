#pragma once
#include <Vector>
#include <QString>

struct Point3D {
	float x, y, z;
};

struct Triangle {
	int idx[3];
};

class ThreeDobj {
private:
    std::vector<Point3D> vertexes;
    std::vector<Triangle> faces;

public:
    ThreeDobj() {}

    void createCube(double size);
    void createUVSphere(int parallels, int meridians, double radius);

    bool saveVtk(const QString& filename);
    bool loadVtk(const QString& filename);

    void clear() {
        vertexes.clear();
        faces.clear();
    }
};