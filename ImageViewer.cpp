#include "ImageViewer.h"
#include <cmath>

ImageViewer::ImageViewer(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::ImageViewerClass)
{
	ui->setupUi(this);
	vW = new ViewerWidget(QSize(500, 500), ui->scrollArea);
	ui->scrollArea->setWidget(vW);

	ui->scrollArea->setBackgroundRole(QPalette::Dark);
	ui->scrollArea->setWidgetResizable(false);
	ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	vW->setObjectName("ViewerWidget");
	vW->installEventFilter(this);

	ui->toolButtonDrawLine->setCheckable(true);
	ui->toolButtonDrawCircle->setCheckable(true);
	ui->toolButtonDrawPoly->setCheckable(true);


	globalColor = Qt::blue;
	QString style_sheet = QString("background-color: %1;").arg(globalColor.name(QColor::HexRgb));
	ui->pushButtonSetColor->setStyleSheet(style_sheet);

	connect(ui->spinBoxAngle, QOverload<int>::of(&QSpinBox::valueChanged), this, &ImageViewer::on_drawCurve_triggered);
	connect(ui->spinBoxLength, QOverload<int>::of(&QSpinBox::valueChanged), this, &ImageViewer::on_drawCurve_triggered);
	connect(ui->comboBoxCurve, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ImageViewer::on_drawCurve_triggered);
	connect(ui->checkBoxCurve, &QCheckBox::stateChanged, this, &ImageViewer::on_drawCurve_triggered);

	connect(ui->createCube_btn, &QPushButton::clicked, this, &ImageViewer::on_createCube_btn_clicked);
	connect(ui->createSphere_btn, &QPushButton::clicked, this, &ImageViewer::on_createSphere_btn_clicked);
	connect(ui->actionSave_Vtk, &QAction::triggered, this, &ImageViewer::on_actionSave_VTK_triggered);
	connect(ui->actionLoad_Vtk, &QAction::triggered, this, &ImageViewer::on_actionLoad_VTK_triggered);
}

// Event filters
bool ImageViewer::eventFilter(QObject* obj, QEvent* event)
{
	if (obj->objectName() == "ViewerWidget") {
		return ViewerWidgetEventFilter(obj, event);
	}
	return QMainWindow::eventFilter(obj, event);
}

//ViewerWidget Events
bool ImageViewer::ViewerWidgetEventFilter(QObject* obj, QEvent* event)
{
	ViewerWidget* w = static_cast<ViewerWidget*>(obj);

	if (!w) {
		return false;
	}

	if (event->type() == QEvent::MouseButtonPress) {
		ViewerWidgetMouseButtonPress(w, event);
	}
	else if (event->type() == QEvent::MouseButtonRelease) {
		ViewerWidgetMouseButtonRelease(w, event);
	}
	else if (event->type() == QEvent::MouseMove) {
		ViewerWidgetMouseMove(w, event);
	}
	else if (event->type() == QEvent::Leave) {
		ViewerWidgetLeave(w, event);
	}
	else if (event->type() == QEvent::Enter) {
		ViewerWidgetEnter(w, event);
	}
	else if (event->type() == QEvent::Wheel) {
		ViewerWidgetWheel(w, event);
	}

	return QObject::eventFilter(obj, event);
}

void ImageViewer::ViewerWidgetMouseButtonPress(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);

	// Ë²Í²ß
	if (ui->toolButtonDrawLine->isChecked() && e->button() == Qt::LeftButton) {
		if (!w->getDrawLineActivated() && ui->radioButtonClear->isChecked()) {
			w->clear();
		}

		if (w->getDrawLineActivated()) {
			w->drawLine(w->getDrawLineBegin(), e->pos(), globalColor, ui->comboBoxLineAlg->currentIndex());
			w->setDrawLineActivated(false);
		}
		else {
			w->setDrawLineBegin(e->pos());
			w->setDrawLineActivated(true);
			w->setPixel(e->pos().x(), e->pos().y(), globalColor);
			w->update();
		}
	}
	// ÊÎËÎ
	else if (ui->toolButtonDrawCircle->isChecked() && e->button() == Qt::LeftButton) {
		if (!w->getDrawCircleActivated() && ui->radioButtonClear->isChecked()) {
			w->clear();
		}

		if (w->getDrawCircleActivated()) {
			QPoint center = w->getDrawCircleBegin();
			int radius = static_cast<int>(std::hypot(e->pos().x() - center.x(), e->pos().y() - center.y())); //ðîçðàõóíêó ðàä³ó
			w->drawCircleBresenham(center, radius, globalColor);
			w->setDrawCircleActivated(false);
			w->update();
		}
		else {
			w->setDrawCircleBegin(e->pos());
			w->setDrawCircleActivated(true);
			w->setPixel(e->pos().x(), e->pos().y(), globalColor);
			w->update();
		}
	}
	// ÏÎË²ÃÎÍ
	else if (ui->toolButtonDrawPoly->isChecked()) {
		if (e->button() == Qt::LeftButton) {
			if (w->getIsPolygonClosed()) {
				if (ui->radioButtonClear->isChecked()) {
					w->clear();
				}
				else {
					w->clearPolygon();
				}
			}

			w->addPoint(e->pos());

			if (ui->radioButtonClear->isChecked()) {
				w->clearCanvas();
			}

			w->drawCurrentObject(globalColor);
			w->update();
		}
		else if (e->button() == Qt::RightButton) {
			w->closePath();
			w->drawCurrentObject(globalColor);
			w->update();
		}
	}
	// ÊÐÈÂÀ
	else if (ui->toolButtonDrawCurve->isChecked() && e->button() == Qt::LeftButton) {
		w->addPoint(e->pos()); 
		w->setPixel(e->pos().x(), e->pos().y(), Qt::black);

		on_drawCurve_triggered();
		w->update();
	}
	// ÏÅÐÅÌ²ÙÅÍÍß
	else if (e->button() == Qt::LeftButton) {
		w->setDragging(true);
		w->setLastMousePos(e->pos());
	}
}

void ImageViewer::ViewerWidgetMouseButtonRelease(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);

	if (e->button() == Qt::LeftButton) {
		w->setDragging(false);
	}
}
void ImageViewer::ViewerWidgetMouseMove(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);

	if (w->getIsDragging()) {
		QPoint delta = e->pos() - w->getLastMousePos();
		w->moveObject(delta, globalColor);
		w->setLastMousePos(e->pos());
	}

}
void ImageViewer::ViewerWidgetLeave(ViewerWidget* w, QEvent* event)
{
}
void ImageViewer::ViewerWidgetEnter(ViewerWidget* w, QEvent* event)
{
}
void ImageViewer::ViewerWidgetWheel(ViewerWidget* w, QEvent* event)
{
	QWheelEvent* e = static_cast<QWheelEvent*>(event);

	double factor = (e->angleDelta().y() > 0) ? 1.25 : 0.75;

	if (w->getPointsCount() > 0) {
		w->scaleObject(factor, factor, globalColor);
	}
}

//ImageViewer Events
void ImageViewer::closeEvent(QCloseEvent* event)
{
	if (QMessageBox::Yes == QMessageBox::question(this, "Close Confirmation", "Are you sure you want to exit?", QMessageBox::Yes | QMessageBox::No))
	{
		event->accept();
	}
	else {
		event->ignore();
	}
}

//Image functions
bool ImageViewer::openImage(QString filename)
{
	QImage loadedImg(filename);
	if (!loadedImg.isNull()) {
		return vW->setImage(loadedImg);
	}
	return false;
}
bool ImageViewer::saveImage(QString filename)
{
	QFileInfo fi(filename);
	QString extension = fi.completeSuffix();

	QImage* img = vW->getImage();
	return img->save(filename, extension.toStdString().c_str());
}

//Slots
void ImageViewer::on_actionOpen_triggered()
{
	QString folder = settings.value("folder_img_load_path", "").toString();

	QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm *.xbm *.xpm);;All files (*)";
	QString fileName = QFileDialog::getOpenFileName(this, "Load image", folder, fileFilter);
	if (fileName.isEmpty()) { return; }

	QFileInfo fi(fileName);
	settings.setValue("folder_img_load_path", fi.absoluteDir().absolutePath());

	if (!openImage(fileName)) {
		msgBox.setText("Unable to open image.");
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.exec();
	}
}
void ImageViewer::on_actionSave_as_triggered()
{
	QString folder = settings.value("folder_img_save_path", "").toString();

	QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm *.xbm *.xpm);;All files (*)";
	QString fileName = QFileDialog::getSaveFileName(this, "Save image", folder, fileFilter);
	if (!fileName.isEmpty()) {
		QFileInfo fi(fileName);
		settings.setValue("folder_img_save_path", fi.absoluteDir().absolutePath());

		if (!saveImage(fileName)) {
			msgBox.setText("Unable to save image.");
			msgBox.setIcon(QMessageBox::Warning);
		}
		else {
			msgBox.setText(QString("File %1 saved.").arg(fileName));
			msgBox.setIcon(QMessageBox::Information);
		}
		msgBox.exec();
	}
}
void ImageViewer::on_pushButtonClear_clicked()
{
	vW->clear();
}
void ImageViewer::on_actionExit_triggered()
{
	this->close();
}


void ImageViewer::on_pushButtonSetColor_clicked()
{
	QColor newColor = QColorDialog::getColor(globalColor, this);
	if (newColor.isValid()) {
		QString style_sheet = QString("background-color: %1;").arg(newColor.name(QColor::HexRgb));
		ui->pushButtonSetColor->setStyleSheet(style_sheet);
		globalColor = newColor;
	}
}

void ImageViewer::on_toolButtonRotate_clicked()
{
	double angle = static_cast<double>(ui->spinBoxRotate->value());

	if (angle == 0) return;
	vW->rotateObject(angle, globalColor);
	vW->applyRotation();
	ui->toolButtonRotate->clearFocus();
}

// ìàñøòàáóâàííÿ
void ImageViewer::on_toolButtonScale_clicked() {
	double sx = ui->doubleSpinBoxScaleX->value();
	double sy = ui->doubleSpinBoxScaleY->value();
	if (sx != 0 && sy != 0) {
		vW->scaleObject(sx, sy, globalColor);
	}
}

// íàõèë
void ImageViewer::on_toolButtonSlope_clicked() {
	double shx = ui->doubleSpinBoxSlopeX->value();
	vW->slopeObject(shx, globalColor);
}

// ñèìåòð³ÿ
void ImageViewer::on_toolButtonSymmetry_clicked() {
	vW->symmetryObject(globalColor);
}

// vertexy
void ImageViewer::on_pushButtonVertex1_clicked() {
	QColor c = QColorDialog::getColor(v1Color, this, "1 vertex color");
	if (c.isValid()) {
		v1Color = c;
		ui->pushButtonVertex1->setStyleSheet(QString("background-color: %1").arg(c.name()));
	}
}

void ImageViewer::on_pushButtonVertex2_clicked() {
	QColor c = QColorDialog::getColor(v2Color, this, "2 vertex color");
	if (c.isValid()) {
		v2Color = c;
		ui->pushButtonVertex2->setStyleSheet(QString("background-color: %1").arg(c.name()));
	}
}

void ImageViewer::on_pushButtonVertex3_clicked() {
	QColor c = QColorDialog::getColor(v3Color, this, "3 vertex color");
	if (c.isValid()) {
		v3Color = c;
		ui->pushButtonVertex3->setStyleSheet(QString("background-color: %1").arg(c.name()));
	}
}

void ImageViewer::on_pushButtonFill_clicked() {
	int method = ui->comboBoxInterpolation->currentIndex();
	if (vW->getPointsCount() < 3) return;
	vW->fillV1 = v1Color;
	vW->fillV2 = v2Color;
	vW->fillV3 = v3Color;
	vW->lastFillMethod = method;

	vW->clearCanvas();

	for (int i = 0; i < vW->getPointsCount(); i++) {
		vW->drawLineBresenham(vW->getPoint(i), vW->getPoint((i + 1) % vW->getPointsCount()), globalColor);
	}

	if (method == 0) {
		vW->fillPolygonScanLine(globalColor);
	}
	else {
		// Äëÿ Neighbors (1) òà Barycentric (2) âèêëèêàºìî ³íòåðïîëÿö³þ
		vW->applyLastFill();
	}

	vW->update();
}

void ImageViewer::on_drawCurve_triggered() {
	vW->clearCanvas();

	if (ui->checkBoxCurve->isChecked()) {
		for (int i = 0; i < vW->getPointsCount(); ++i) {
			vW->drawCircleBresenham(vW->getPoint(i), 4, Qt::red);
			if (i < vW->getPointsCount() - 1) {
				vW->drawLine(vW->getPoint(i), vW->getPoint(i + 1), Qt::gray, 0);
			}
		}
	}

	int type = ui->comboBoxCurve->currentIndex();
	if (vW->getPointsCount() < 2) return;

	if (type == 0) { // Hermite
		vW->drawHermite(globalColor, ui->spinBoxAngle->value(), ui->spinBoxLength->value());
	}
	else if (type == 1) { // Bezier
		vW->drawBezier(globalColor);
	}
	else if (type == 2 && vW->getPointsCount() >= 4) { // B-Spline
		vW->drawBSpline(globalColor);
	}
	vW->update();
}


///////////////// 3D

void ImageViewer::on_actionSave_VTK_triggered() {
	QString fileName = QFileDialog::getSaveFileName(this, "Save VTK", "", "VTK Files (*.vtk)");

	if (!fileName.isEmpty()) {
		model.saveVtk(fileName);
	}
}

void ImageViewer::on_actionLoad_VTK_triggered() {
	QString fileName = QFileDialog::getOpenFileName(this, "Load VTK", "", "VTK Files (*.vtk)");

	if (!fileName.isEmpty()) {
		model.loadVtk(fileName);
		vW->update();
	}
}

void ImageViewer::on_createCube_btn_clicked() {
	qDebug() << "Button clicked!";
	double size = ui->side_sb->value();

	model.createCube(size);

	vW->update();
}

void ImageViewer::on_createSphere_btn_clicked() {
	qDebug() << "Button clicked!";	
	double r = ui->radius_sb->value();
	int p = ui->paral_sb->value();
	int m = ui->merid_sb->value();

	model.createUVSphere(p, m, r);

	vW->update();
}

void ImageViewer::on_pushButtonClearPoints_clicked() {
	vW->clearPolygon();
	vW->clearCanvas();
	vW->update();
}