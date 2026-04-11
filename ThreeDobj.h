#pragma once
#include <QVector3D>
#include <QVector>
#include <QString>

class ThreeDobj {
private:
	QVector<QVector3D> vertexes;
	QVector<QVector<int>> faces;
public:
	ThreeDobj() {}
	~ThreeDobj() {}
	bool saveVtk(const QString& filename) const;
	bool loadVtk(const QString& filename);
	void clear() { }
};