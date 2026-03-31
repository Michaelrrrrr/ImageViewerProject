#pragma once
#include <QtWidgets>
class ViewerWidget :public QWidget {
	Q_OBJECT
private:
	QSize areaSize = QSize(0, 0); //rozmery polotna
	QImage* img = nullptr; // ma datovu strukturu a v nej ulozeny smernik s.....
	uchar* data = nullptr;

	bool drawLineActivated = false;
	QPoint drawLineBegin = QPoint(0, 0);

	QPoint circleBegin;//stred kruznice
	bool isCircleActivated = false;// Чи чекаємо ми на другий клік (радіус)

	QVector<QPoint> polyPoints; // Точки активного об'єкта
	bool isPolygonClosed = false;
	bool isPolygonActivated = false; // Чи чекаємо ми на клік для замикання полігону

	QVector<QPoint> originalPoints;

	bool isDragging = false;
	QPoint lastMousePos;

	enum OutCode { INSIDE = 0, LEFT = 1, RIGHT = 2, BOTTOM = 4, TOP = 8};


public:
	ViewerWidget(QSize imgSize, QWidget* parent = Q_NULLPTR);
	~ViewerWidget();
	void resizeWidget(QSize size);

	//Image functions
	bool setImage(const QImage& inputImg);
	QImage* getImage() { return img; };
	bool isEmpty();
	bool changeSize(int width, int height);

	void setPixel(int x, int y, int r, int g, int b, int a = 255);
	void setPixel(int x, int y, double valR, double valG, double valB, double valA = 1.);
	void setPixel(int x, int y, const QColor& color);
	bool isInside(int x, int y);

	//Draw functions
	void drawLine(QPoint start, QPoint end, QColor color, int algType = 0);
	void setDrawLineBegin(QPoint begin) { drawLineBegin = begin; }
	QPoint getDrawLineBegin() { return drawLineBegin; }
	void setDrawLineActivated(bool state) { drawLineActivated = state; }
	bool getDrawLineActivated() { return drawLineActivated; }

	void setDrawCircleActivated(bool state) { isCircleActivated = state; }
	bool getDrawCircleActivated() { return isCircleActivated; }

	void closePath() { if (polyPoints.size() > 2) isPolygonClosed = true; }
	void drawCurrentObject(QColor color);

	void moveObject(QPoint delta, QColor color);

	void rotateObject(double degrees, QColor color);

	void applyRotation();

	//Get/Set functions
	uchar* getData() { return data; }
	void setDataPtr() { data = img ? img->bits() : nullptr; } //zoberie data a priradi smerniku

	int getImgWidth() { return img ? img->width() : 0; };
	int getImgHeight() { return img ? img->height() : 0; };

	//Algorithms
	void drawLineDDA(QPoint start, QPoint end, QColor color);
	void drawLineBresenham(QPoint start, QPoint end, QColor color);

	void drawCircleBresenham(QPoint center, int radius, QColor color);
	void setCircleBresenhamActivated(bool state) { isCircleActivated = state; }

	void setDrawCircleBegin(QPoint p) { circleBegin = p; }
	QPoint getDrawCircleBegin() { return circleBegin; }

	bool getIsPolygonClosed() { return isPolygonClosed; }
	void clearPolygon();

	void clear();
	void addPoint(QPoint p);
	void clearCanvas();

	// Методи посуву 
	void setDragging(bool s) { isDragging = s; }
	bool getIsDragging() { return isDragging; }
	void setLastMousePos(QPoint p) { lastMousePos = p; }
	QPoint getLastMousePos() { return lastMousePos; }

	void scaleObject(double sx, double sy, QColor color);
	void slopeObject(double shx, QColor color);
	void symmetryObject(QColor color);
	QPoint getPoint(int i);


	int computeOutCode(double x, double y);
	bool clipLine(QPoint& p1, QPoint& p2); // Коен-Сазерленд
	QVector<QPoint> clipPolygon(const QVector<QPoint>& points); // Сазерленд-Ходжмен

	void fillPolygonScanLine(QColor color);
	void fillTriangle(QPoint p1, QPoint p2, QPoint p3, 
		QColor c1, QColor c2, QColor c3, int method); // 0 Neighbors, 1 Barycentrick
	void getBarycentric(QPoint p, QPoint p1, QPoint p2, QPoint p3, double& l0, double& l1, double& l2);
	QVector<QPoint> getPoints() { return polyPoints; }
	int getPointsCount() { return polyPoints.size(); }

	int lastFillMethod = -1;
	QColor fillV1 = Qt::red;
	QColor fillV2 = Qt::green;
	QColor fillV3 = Qt::blue;
	// Допоміжна функція для обрізання
	QPoint intersect(QPoint p1, QPoint p2, int edge, int limit);
	
	void applyLastFill();

	struct HermiteData {
		double angle = 0;
		double length = 150;
		HermiteData() : angle(0), length(150) {}
	};
	void drawHermite(QColor color, double angle, double length);
	void drawBezier(QColor color);
	void drawBSpline(QColor color);

public slots:
	void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
};