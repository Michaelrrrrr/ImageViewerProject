#include   "ViewerWidget.h"
#include "ImageViewer.h"

ViewerWidget::ViewerWidget(QSize imgSize, QWidget* parent)
	: QWidget(parent)
{
	setAttribute(Qt::WA_StaticContents);
	setMouseTracking(true);
	if (imgSize != QSize(0, 0)) {
		img = new QImage(imgSize, QImage::Format_ARGB32);
		img->fill(Qt::white);
		resizeWidget(img->size());
		setDataPtr();
	}
}
ViewerWidget::~ViewerWidget()
{
	delete img;
	img = nullptr;
	data = nullptr;
}
void ViewerWidget::resizeWidget(QSize size)
{
	this->resize(size);
	this->setMinimumSize(size);
	this->setMaximumSize(size);
}

//Image functions
bool ViewerWidget::setImage(const QImage& inputImg)
{
	if (img) {
		delete img;
		img = nullptr;
		data = nullptr;
	}
	img = new QImage(inputImg.convertToFormat(QImage::Format_ARGB32));
	if (!img || img->isNull()) {
		return false;
	}
	resizeWidget(img->size());
	setDataPtr();
	update();

	return true;
}
bool ViewerWidget::isEmpty()
{
	if (img == nullptr) {
		return true;
	}

	if (img->size() == QSize(0, 0)) {
		return true;
	}
	return false;
}

bool ViewerWidget::changeSize(int width, int height)
{
	QSize newSize(width, height);

	if (newSize != QSize(0, 0)) {
		if (img != nullptr) {
			delete img;
		}

		img = new QImage(newSize, QImage::Format_ARGB32);
		if (!img || img->isNull()) {
			return false;
		}
		img->fill(Qt::white);
		resizeWidget(img->size());
		setDataPtr();
		update();
	}

	return true;
}

void ViewerWidget::setPixel(int x, int y, int r, int g, int b, int a)
{
	if (!img || !data) return;
	if (!isInside(x, y)) return; 

	r = r > 255 ? 255 : (r < 0 ? 0 : r);
	g = g > 255 ? 255 : (g < 0 ? 0 : g);
	b = b > 255 ? 255 : (b < 0 ? 0 : b);
	a = a > 255 ? 255 : (a < 0 ? 0 : a);
	// Прямий доступ до пам'яті, для малювання
	size_t startbyte = y * img->bytesPerLine() + x * 4;
	data[startbyte] = static_cast<uchar>(b);
	data[startbyte + 1] = static_cast<uchar>(g);
	data[startbyte + 2] = static_cast<uchar>(r);
	data[startbyte + 3] = static_cast<uchar>(a);
}
void ViewerWidget::setPixel(int x, int y, double valR, double valG, double valB, double valA)
{
	valR = valR > 1 ? 1 : (valR < 0 ? 0 : valR);
	valG = valG > 1 ? 1 : (valG < 0 ? 0 : valG);
	valB = valB > 1 ? 1 : (valB < 0 ? 0 : valB);
	valA = valA > 1 ? 1 : (valA < 0 ? 0 : valA);

	setPixel(x, y, static_cast<int>(255 * valR + 0.5), static_cast<int>(255 * valG + 0.5), static_cast<int>(255 * valB + 0.5), static_cast<int>(255 * valA + 0.5));
}
void ViewerWidget::setPixel(int x, int y, const QColor& color)
{
	if (color.isValid()) {
		setPixel(x, y, color.red(), color.green(), color.blue(), color.alpha());
	}
}

bool ViewerWidget::isInside(int x, int y)
{
	return img && x >= 0 && y >= 0 && x < img->width() && y < img->height();
}

//Draw functions
void ViewerWidget::drawLine(QPoint start, QPoint end, QColor color, int algType)
{
	if (!img || !data) return;

	/*if (algType == 0) {
		drawLineDDA(start, end, color);
	}
	else {
		drawLineBresenham(start, end, color);
	}
	update();*/

	//Po implementovani drawLineDDA a drawLineBresenham treba vymazat
	QPainter painter(img);
	painter.setPen(QPen(color));
	painter.drawLine(start, end);
	update();
}

void ViewerWidget::addPoint(QPoint p) {
	polyPoints.append(p);
	originalPoints.clear(); // скид. кеш повороту
}

void ViewerWidget::clearCanvas()
{
	if (!img) return;
	img->fill(Qt::white);
	update();
}


void ViewerWidget::clearPolygon()
{
	polyPoints.clear();
	originalPoints.clear(); // Очищ.кеш ротації
	isPolygonClosed = false;
	update();
}

void ViewerWidget::clear() {
    if (!img) return;
    img->fill(Qt::white);
    polyPoints.clear();
    originalPoints.clear(); // Очищ.кеш ротації
    isPolygonClosed = false;
    drawLineActivated = false;
    isCircleActivated = false;
    update();
}
void ViewerWidget::drawLineDDA(QPoint start, QPoint end, QColor color)
{
	double x1 = start.x();
	double y1 = start.y();
	double x2 = end.x();
	double y2 = end.y();

	float dx = x2 - x1;
	float dy = y2 - y1;

	int step = std::max(std::abs(dx), std::abs(dy));

	if(step == 0) {
		setPixel((int)(x1), (int)(y1), color);
		return;
	}

	float incX = dx / step;
	float incY = dy / step;

	for (int i = 0; i <= step; i++) {
		setPixel((int)(x1 + 0.5), (int)(y1 + 0.5), color);
		x1 += incX;
		y1 += incY;
	}
}

void ViewerWidget::drawLineBresenham(QPoint start, QPoint end, QColor color)
{
	int x1 = start.x();
	int y1 = start.y();
	int x2 = end.x();
	int y2 = end.y();

	int dx = abs(x2 - x1); // довжина проекцій
	int dy = abs(y2 - y1);
	int sx = (x1 < x2) ? 1 : -1; //вперед чи назад
	int sy = (y1 < y2) ? 1 : -1;
	int err = dx - dy; //коли нам треба змінити координату

	while (true) {
		setPixel(x1, y1, color);
		if (x1 == x2 && y1 == y2) break;
		int e2 = 2 * err;
		if (e2 > -dy) { //крок по X
			err -= dy;
			x1 += sx;
		}
		if (e2 < dx) {//крок по Y
			err += dx;
			y1 += sy;
		}
	}
}
void ViewerWidget::drawCircleBresenham(QPoint center, int radius, QColor color)
{
	int x = 0;
	int y = radius;
	int d = 3 - 2 * radius; //параметр рішення
	while (x <= y) {
		setPixel(center.x() + x, center.y() + y, color);
		setPixel(center.x() - x, center.y() + y, color);
		setPixel(center.x() + x, center.y() - y, color);
		setPixel(center.x() - x, center.y() - y, color);
		setPixel(center.x() + y, center.y() + x, color);
		setPixel(center.x() - y, center.y() + x, color);
		setPixel(center.x() + y, center.y() - x, color);
		setPixel(center.x() - y, center.y() - x, color);
		if (d < 0) {
			d += 4 * x + 6; // гориз
		}
		else {
			d += 4 * (x - y) + 10; // діаг
			y--;
		}
		x++;
	}
}

//polygon
void ViewerWidget::drawCurrentObject(QColor color) {
	if (polyPoints.size() < 2) return;

	if (isPolygonClosed) { // cпоч кліпуємо, а потім мал
		QVector<QPoint> clipped = clipPolygon(polyPoints);
		for (int i = 0; i < clipped.size(); ++i) { // З'єд
			drawLineBresenham(clipped[i], clipped[(i + 1) % clipped.size()], color);
		}
	}
	else {//незамк
		for (int i = 0; i < polyPoints.size() - 1; ++i) {
			QPoint p1 = polyPoints[i];
			QPoint p2 = polyPoints[i + 1];
			if (clipLine(p1, p2)) { //Відсік, якщ виход
				drawLineBresenham(p1, p2, color);
			}
		}
	}
}

//posun
void ViewerWidget::moveObject(QPoint delta, QColor color) {
	if (polyPoints.isEmpty()) return;

	for (int i = 0; i < polyPoints.size(); ++i) {
		polyPoints[i] += delta;
	}

	clearCanvas();

	// перемалюв. заливки
	if (lastFillMethod == 0) fillPolygonScanLine(color);
	else if (lastFillMethod == 1 || lastFillMethod == 2) {
		// Якщо точок 3, викликаємо тріангл-філ з новими коорд
		if (polyPoints.size() == 3) {
			fillTriangle(polyPoints[0], polyPoints[1], polyPoints[2], fillV1, fillV2, fillV3, lastFillMethod);
		}
	}

	drawCurrentObject(color);
	update();
}

void ViewerWidget::rotateObject(double degrees, QColor color) {
	if (polyPoints.size() < 2) return;
	if (originalPoints.isEmpty()) {//форма не злам
		originalPoints = polyPoints;
	}

	QPoint center = originalPoints.first();
	double radians = degrees * M_PI / 180.0;
	double cosTheta = std::cos(radians);
	double sinTheta = std::sin(radians);
	for (int i = 0; i < originalPoints.size(); ++i) {
		if (i == 0) {
			polyPoints[i] = originalPoints[i];
			continue;
		}

		int dx = originalPoints[i].x() - center.x(); // Вектор
		int dy = originalPoints[i].y() - center.y();

		int newX = center.x() + qRound(dx * cosTheta - dy * sinTheta);
		int newY = center.y() + qRound(dx * sinTheta + dy * cosTheta);

		polyPoints[i] = QPoint(newX, newY);
	}
	clearCanvas();
	drawCurrentObject(color);
	applyLastFill();
	update();
}

void ViewerWidget::applyRotation() {
    if (!polyPoints.isEmpty()) {
        originalPoints = polyPoints;
    }
}


// Масштабування
void ViewerWidget::scaleObject(double sx, double sy, QColor color) {
	if (polyPoints.isEmpty()) return;
	QPoint S = polyPoints.first();
	for (QPoint& p : polyPoints) {
		p.setX(S.x() + qRound((p.x() - S.x()) * sx));
		p.setY(S.y() + qRound((p.y() - S.y()) * sy));
	}
	clearCanvas();
	for (int i = 0; i < polyPoints.size(); i++) {
		drawLineBresenham(polyPoints[i], polyPoints[(i + 1) % polyPoints.size()], color);
	}

	applyLastFill();
	update();
}

// Нахил
void ViewerWidget::slopeObject(double shx, QColor color) {
	if (polyPoints.isEmpty()) return;
	QPoint S = polyPoints.first();
	for (QPoint& p : polyPoints) {
		int dx = p.x() - S.x();
		int dy = p.y() - S.y();
		p.setX(S.x() + qRound(dx + shx * dy));
	}
	clearCanvas();
	drawCurrentObject(color);
	applyLastFill();
	update();
}

// Симетрія
QPoint ViewerWidget::getPoint(int i) {
	if (i >= 0 && i < polyPoints.size()) {
		return polyPoints[i];
	}
	return QPoint(-1, -1);
}

void ViewerWidget::symmetryObject(QColor color) {
	if (polyPoints.size() < 2) return;
	// вісь
	QPoint A = polyPoints[0];
	QPoint B = polyPoints[1];

	double dx = B.x() - A.x();
	double dy = B.y() - A.y();
	double lengthSq = dx * dx + dy * dy;

	if (lengthSq < 0.000000001) return;// Захист
	// трансформуємо
	for (int i = 0; i < polyPoints.size(); ++i) {
		double px = polyPoints[i].x();
		double py = polyPoints[i].y();

		double t = ((px - A.x()) * dx + (py - A.y()) * dy) / lengthSq;
		double projX = A.x() + t * dx;
		double projY = A.y() + t * dy;

		int newX = qRound(2 * projX - px);//протилеж нап
		int newY = qRound(2 * projY - py);

		polyPoints[i] = QPoint(newX, newY);
	}
	clearCanvas();
	for (int i = 0; i < polyPoints.size(); ++i) {
		drawLineBresenham(polyPoints[i], polyPoints[(i + 1) % polyPoints.size()], color);
	}
	applyLastFill();
	update();
}

int ViewerWidget::computeOutCode(double x, double y) {//Коен-Cap
	int code = INSIDE;
	if (x < 0) code |= LEFT;
	else if (x >= img->width()) code |= RIGHT;
	if (y < 0) code |= TOP;
	else if (y >= img->height()) code |= BOTTOM;
	return code;
}

bool ViewerWidget::clipLine(QPoint& p1, QPoint& p2) {
	double x1 = p1.x(), y1 = p1.y(), x2 = p2.x(), y2 = p2.y();
	int code1 = computeOutCode(x1, y1);
	int code2 = computeOutCode(x2, y2);
	bool accept = false;

	while (true) {
		if (!(code1 | code2)) { accept = true; break; }//повністю видима
		else if (code1 & code2) { break; }//невидима
		else {//частково
			int codeOut = code1 ? code1 : code2;
			double x, y;
			if (codeOut & TOP) {//подібність трикутників
				x = x1 + (x2 - x1) * (0 - y1) / (y2 - y1);
				y = 0;
			}
			else if (codeOut & BOTTOM) {
				x = x1 + (x2 - x1) * (img->height() - 1 - y1) / (y2 - y1);
				y = img->height() - 1;
			}
			else if (codeOut & RIGHT) {
				y = y1 + (y2 - y1) * (img->width() - 1 - x1) / (x2 - x1);
				x = img->width() - 1;
			}
			else if (codeOut & LEFT) {
				y = y1 + (y2 - y1) * (0 - x1) / (x2 - x1);
				x = 0;
			}
			//онов коор
			if (codeOut == code1) { x1 = x; y1 = y; code1 = computeOutCode(x1, y1); }
			else { x2 = x; y2 = y; code2 = computeOutCode(x2, y2); }
		}
	}
	if (accept) { p1 = QPoint(qRound(x1), qRound(y1)); p2 = QPoint(qRound(x2), qRound(y2)); }//онов об
	return accept;
}

QVector<QPoint> ViewerWidget::clipPolygon(const QVector<QPoint>& points) {
	QVector<QPoint> output = points;
	int edges[] = { LEFT, RIGHT, TOP, BOTTOM };

	for (int edge : edges) {//межa екрана
		QVector<QPoint> input = output;
		output.clear();
		if (input.isEmpty()) break;

		int limit = 0;
		if (edge == LEFT) limit = 0;
		else if (edge == RIGHT) limit = img->width() - 1;
		else if (edge == TOP) limit = 0;
		else if (edge == BOTTOM) limit = img->height() - 1;

		QPoint s = input.last();
		for (QPoint v : input) {
			bool v_in = !(computeOutCode(v.x(), v.y()) & edge);
			bool s_in = !(computeOutCode(s.x(), s.y()) & edge);

			// ЛОГІКА САЗЕРЛЕНДА-ХОДЖМЕНА
			if (s_in && v_in) {
				// обидві всередині — додаємо тільки поточну точку V
				output.append(v);
			}
			else if (s_in && !v_in) {
				// виходимо назовні — додаємо тільки точку перетину з межею
				output.append(intersect(s, v, edge, limit));
			}
			else if (!s_in && v_in) {
				// входимо всередину — додаємо точку перетину ТА саму точку V
				output.append(intersect(s, v, edge, limit));
				output.append(v);
			}

			s = v;
		}
	}
	return output;
}

QPoint ViewerWidget::intersect(QPoint p1, QPoint p2, int edge, int limit) {
	double x, y;
	if (edge == LEFT || edge == RIGHT) { // вертикальна межа
		x = limit;
		if (p2.x() == p1.x()) return p1; // Захист 0
		y = p1.y() + (p2.y() - p1.y()) * (double)(limit - p1.x()) / (p2.x() - p1.x());
	}
	else { // горизонтальна межа
		y = limit;
		if (p2.y() == p1.y()) return p1;
		x = p1.x() + (p2.x() - p1.x()) * (double)(limit - p1.y()) / (p2.y() - p1.y());//лін інтерп
	}
	return QPoint(qRound(x), qRound(y));
}

void ViewerWidget::applyLastFill() {
	if (lastFillMethod == -1 || polyPoints.size() < 3) return;

	if (lastFillMethod == 0) { // ScanLine
		fillPolygonScanLine(fillV1);
	}
	else { // 1 - Барицентрично, 2 - Сусід
		fillTriangle(polyPoints[0], polyPoints[1], polyPoints[2],
			fillV1, fillV2, fillV3, lastFillMethod);
	}
}

void ViewerWidget::fillPolygonScanLine(QColor color) {
	if (polyPoints.size() < 3) return;

	int yMin = img->height();
	int yMax = 0;
	for (const auto& p : polyPoints) {
		if (p.y() < yMin) yMin = p.y();
		if (p.y() > yMax) yMax = p.y();
	}

	if (yMin < 0) yMin = 0;
	if (yMax >= img->height()) yMax = img->height() - 1;

	for (int y = yMin; y <= yMax; ++y) {
		QVector<int> intersections;// Список X
		intersections.reserve(polyPoints.size()); 

		// Перебираємо кожне ребро полігону
		for (int i = 0; i < polyPoints.size(); ++i) {
			QPoint p1 = polyPoints[i];
			QPoint p2 = polyPoints[(i + 1) % polyPoints.size()];

			//ігнор горизон
			if (p1.y() == p2.y()) continue;
			//чи перетинає поточний Y відрізок p1-p2
			if ((y >= p1.y() && y < p2.y()) || (y >= p2.y() && y < p1.y())) {
				double x = p1.x() + (double)(y - p1.y()) * (p2.x() - p1.x()) / (p2.y() - p1.y());//лін інтерп
				intersections.append(qRound(x));
			}
		}

		if (intersections.isEmpty()) continue;

		std::sort(intersections.begin(), intersections.end());

		// Проходимо по відсортованих перетинах парами (ліва, права)
		for (int i = 0; i + 1 < intersections.size(); i += 2) {
			int xStart = intersections[i];
			int xEnd = intersections[i + 1];

			// Захист від некоректних пар
			if (xStart > xEnd) std::swap(xStart, xEnd);

			// Обрізаємо по межах зображення для уникнення зайвих викликів setPixel
			if (xStart < 0) xStart = 0;
			if (xEnd >= img->width()) xEnd = img->width() - 1;

			// Заповнюємо від xStart до xEnd
			for (int x = xStart; x <= xEnd; ++x) {
				// Викликаємо setPixel; метод уже перевіряє межі через isInside
				setPixel(x, y, color.red(), color.green(), color.blue());
			}
		}
	}
	update();
}

void ViewerWidget::getBarycentric(QPoint p, QPoint p1, QPoint p2, QPoint p3,
	double& l0, double& l1, double& l2) {
	double det = (double)(p2.y() - p3.y()) * (p1.x() - p3.x()) + (double)(p3.x() - p2.x()) * (p1.y() - p3.y());
	l0 = ((double)(p2.y() - p3.y()) * (p.x() - p3.x()) + (double)(p3.x() - p2.x()) * (p.y() - p3.y())) / det;
	l1 = ((double)(p3.y() - p1.y()) * (p.x() - p3.x()) + (double)(p1.x() - p3.x()) * (p.y() - p3.y())) / det;
	l2 = 1.0 - l0 - l1;
}


void ViewerWidget::fillTriangle(QPoint p1, QPoint p2, QPoint p3, QColor c1, QColor c2, QColor c3, int method) {
	// Обмежувальна рамка
	int minX = std::min({ p1.x(), p2.x(), p3.x() });
	int maxX = std::max({ p1.x(), p2.x(), p3.x() });
	int minY = std::min({ p1.y(), p2.y(), p3.y() });
	int maxY = std::max({ p1.y(), p2.y(), p3.y() });

	for (int y = minY; y <= maxY; ++y) {
		for (int x = minX; x <= maxX; ++x) {
			double l0, l1, l2;
			getBarycentric(QPoint(x, y), p1, p2, p3, l0, l1, l2);

			// чи точка всередині трикутника
			if (l0 >= 0 && l1 >= 0 && l2 >= 0) {
				if (method == 1) { // NEIGHBOURS
					double d1 = std::pow(x - p1.x(), 2) + std::pow(y - p1.y(), 2);
					double d2 = std::pow(x - p2.x(), 2) + std::pow(y - p2.y(), 2);
					double d3 = std::pow(x - p3.x(), 2) + std::pow(y - p3.y(), 2);

					if (d1 <= d2 && d1 <= d3) setPixel(x, y, c1);
					else if (d2 <= d1 && d2 <= d3) setPixel(x, y, c2);
					else setPixel(x, y, c3);
				}
				else if (method == 2) { // BARYCENTRIC
					int r = qBound(0, qRound(l0 * c1.red() + l1 * c2.red() + l2 * c3.red()), 255);
					int g = qBound(0, qRound(l0 * c1.green() + l1 * c2.green() + l2 * c3.green()), 255);
					int b = qBound(0, qRound(l0 * c1.blue() + l1 * c2.blue() + l2 * c3.blue()), 255);
					setPixel(x, y, QColor(r, g, b));
				}
			}
		}
	}
}

QPointF deCasteljau(const QVector<QPoint>& points, double t) {//розрізання кривої Безьє
	QVector<QPointF> temp;
	for (const auto& p : points) temp.push_back(p);
	// Поступово зменшуємо кількість точок, знаходячи проміжні координати
	for (int k = 1; k < temp.size(); ++k) {
		for (int i = 0; i < temp.size() - k; ++i) {
			temp[i] = (1 - t) * temp[i] + t * temp[i + 1];//лін інтерп
		}
	}
	return temp[0];
}

void ViewerWidget::drawHermite(QColor color, double angle, double length) {
	if (polyPoints.size() < 2) return;

	double rad = qDegreesToRadians(angle);
	// Вектор дотичної
	QPointF T(std::cos(rad) * length, std::sin(rad) * length);
	for (const QPoint& p : polyPoints) {
		drawLineBresenham(p, (p + T.toPoint()), Qt::gray);
	}
	for (int i = 0; i < polyPoints.size() - 1; ++i) {
		QPointF P0 = polyPoints[i];
		QPointF P1 = polyPoints[i + 1];
		QPointF prev = P0;

		for (double t = 0.0; t <= 1.0; t += 0.005) {
			double t2 = t * t;
			double t3 = t2 * t;

			double h1 = 2 * t3 - 3 * t2 + 1;
			double h2 = -2 * t3 + 3 * t2;
			double h3 = t3 - 2 * t2 + t;
			double h4 = t3 - t2;
		//сума впливу точок і дотичних
			QPointF curr = h1 * P0 + h2 * P1 + h3 * T + h4 * T;

			drawLineBresenham(prev.toPoint(), curr.toPoint(), color);
			prev = curr;
		}
	}
}

void ViewerWidget::drawBezier(QColor color) {
	if (polyPoints.size() < 2) return;

	auto deCasteljau = [&](double t) mutable -> QPointF {
		QVector<QPointF> tmp;
		for (const auto& p : polyPoints) tmp.push_back(p);
		for (int k = 1; k < tmp.size(); ++k) {
			for (int j = 0; j < tmp.size() - k; ++j) {
				tmp[j] = (1 - t) * tmp[j] + t * tmp[j + 1];
			}
		}
		return tmp[0];
		};

	QPointF prev = polyPoints[0];
	for (double t = 0; t <= 1.0; t += 0.01) {
		QPointF curr = deCasteljau(t);
		drawLineBresenham(prev.toPoint(), curr.toPoint(), color);
		prev = curr;
	}
}

void ViewerWidget::drawBSpline(QColor color) {
	if (polyPoints.size() < 4) return; 
	for (int i = 0; i < polyPoints.size() - 3; ++i) {
		QPoint p0 = polyPoints[i];
		QPoint p1 = polyPoints[i + 1];
		QPoint p2 = polyPoints[i + 2];
		QPoint p3 = polyPoints[i + 3];

		QPointF prevPoint;
		bool first = true;

		for (double t = 0; t <= 1.0 + 0.0001; t += 0.01) {
			double t2 = t * t;
			double t3 = t2 * t;
			// баз фун
			double b0 = (-t3 + 3 * t2 - 3 * t + 1.0) / 6.0;
			double b1 = (3 * t3 - 6 * t2 + 4.0) / 6.0;
			double b2 = (-3 * t3 + 3 * t2 + 3 * t + 1.0) / 6.0;
			double b3 = t3 / 6.0;
			//X та Y як зваженa сумa
			double x = b0 * p0.x() + b1 * p1.x() + b2 * p2.x() + b3 * p3.x();
			double y = b0 * p0.y() + b1 * p1.y() + b2 * p2.y() + b3 * p3.y();
			QPointF currPoint(x, y);

			if (!first) {
				drawLineBresenham(prevPoint.toPoint(), currPoint.toPoint(), color);
			}
			prevPoint = currPoint;
			first = false;
		}
	}
}

//Slots
void ViewerWidget::paintEvent(QPaintEvent * event)
{
	QPainter painter(this);
	if (!img || img->isNull()) return;

	QRect area = event->rect();
	painter.drawImage(area, *img, area);
}