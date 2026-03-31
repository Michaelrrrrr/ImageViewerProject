#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets>
#include "ui_ImageViewer.h"
#include "ViewerWidget.h"

class ImageViewer : public QMainWindow
{
	Q_OBJECT

public:
	ImageViewer(QWidget* parent = Q_NULLPTR);
	~ImageViewer() { delete ui; }

private:
	Ui::ImageViewerClass* ui;
	ViewerWidget* vW;

	QColor globalColor;
	QSettings settings;
	QMessageBox msgBox;

	// Event filters
	bool eventFilter(QObject* obj, QEvent* event);

	// ViewerWidget Events
	bool ViewerWidgetEventFilter(QObject* obj, QEvent* event);
	void ViewerWidgetMouseButtonPress(ViewerWidget* w, QEvent* event);
	void ViewerWidgetMouseButtonRelease(ViewerWidget* w, QEvent* event);
	void ViewerWidgetMouseMove(ViewerWidget* w, QEvent* event);
	void ViewerWidgetLeave(ViewerWidget* w, QEvent* event);
	void ViewerWidgetEnter(ViewerWidget* w, QEvent* event);
	void ViewerWidgetWheel(ViewerWidget* w, QEvent* event); // Шкалювання

	// ImageViewer Events
	void closeEvent(QCloseEvent* event);

	// Image functions
	bool openImage(QString filename);
	bool saveImage(QString filename);

	bool autoClear = true;

	//fill vertexy
	QColor v1Color = Qt::red;
	QColor v2Color = Qt::green;
	QColor v3Color = Qt::blue;

private slots:
	void on_actionOpen_triggered();
	void on_actionSave_as_triggered();
	void on_pushButtonClear_clicked();
	void on_actionExit_triggered();
	void on_pushButtonSetColor_clicked();
	void on_toolButtonRotate_clicked();
	void on_toolButtonScale_clicked();
	void on_toolButtonSlope_clicked();
	void on_toolButtonSymmetry_clicked();

	void on_pushButtonVertex1_clicked();
	void on_pushButtonVertex2_clicked();
	void on_pushButtonVertex3_clicked();
	void on_pushButtonFill_clicked();


	void on_pushButtonClearPoints_clicked();
	void on_drawCurve_triggered();

};