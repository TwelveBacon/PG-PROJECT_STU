#include "ImageViewer.h"

ImageViewer::ImageViewer(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::ImageViewerClass)
{
	ui->setupUi(this);
	ui->comboBoxLineAlg->setEnabled(false);
	ui->toolButtonDrawLine->setChecked(false);
	ui->toolButtonDrawCircle->setChecked(false);
	vW = new ViewerWidget(QSize(500, 500), ui->scrollArea);
	ui->scrollArea->setWidget(vW);

	ui->scrollArea->setBackgroundRole(QPalette::Dark);
	ui->scrollArea->setWidgetResizable(false);
	ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	vW->setObjectName("ViewerWidget");
	vW->installEventFilter(this);

	globalColor = Qt::blue;
	QString style_sheet = QString("background-color: %1;").arg(globalColor.name(QColor::HexRgb));
	ui->pushButtonSetColor->setStyleSheet(style_sheet);
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

	if (e->button() != Qt::LeftButton && e->button() != Qt::RightButton) {
		return;
	}

	// LINE
	if (ui->toolButtonDrawLine->isChecked()) {

		if (e->button() != Qt::LeftButton) {
			return;
		}

		if (w->hasObject() && !w->getDrawLineActivated()) {
			return;
		}

		if (w->getDrawLineActivated()) {
			QVector<QPoint> pts;
			pts.push_back(w->getDrawLineBegin());
			pts.push_back(e->pos());

			w->setObjectPoints(pts);
			w->setCurrentObjectType(ViewerWidget::ObjectLine);

			w->setDrawLineActivated(false);
			w->setShapeFilled(false);
			w->setFillMode(ViewerWidget::FillNone);

			w->redrawScene(globalColor, ui->comboBoxLineAlg->currentIndex());
		}
		else {
			w->setDrawLineBegin(e->pos());
			w->setDrawLineActivated(true);

			w->setObjectPoints(QVector<QPoint>());
			w->setCurrentObjectType(ViewerWidget::ObjectNone);
			w->setShapeFilled(false);
			w->setFillMode(ViewerWidget::FillNone);
		}

		return;
	}

	// CIRCLE
	if (ui->toolButtonDrawCircle->isChecked()) {

		if (e->button() != Qt::LeftButton) {
			return;
		}

		if (w->hasObject() && !w->getDrawCircleActivated()) {
			return;
		}

		if (w->getDrawCircleActivated()) {
			QPoint center = w->getDrawCircleCenter();

			int radius = static_cast<int>(std::round(std::sqrt(
				(center.x() - e->pos().x()) * (center.x() - e->pos().x()) +
				(center.y() - e->pos().y()) * (center.y() - e->pos().y())
			)));

			w->clear();
			w->drawCircle(center, radius, globalColor);

			w->setDrawCircleActivated(false);

			w->setCurrentObjectType(ViewerWidget::ObjectNone);
			w->setObjectPoints(QVector<QPoint>());
			w->setShapeFilled(false);
			w->setFillMode(ViewerWidget::FillNone);
		}
		else {
			w->setDrawCircleCenter(e->pos());
			w->setDrawCircleActivated(true);

			w->setCurrentObjectType(ViewerWidget::ObjectNone);
			w->setObjectPoints(QVector<QPoint>());
			w->setShapeFilled(false);
			w->setFillMode(ViewerWidget::FillNone);
		}

		return;
	}

	// POLYGON / TRIANGLE
	if (ui->toolButtonDrawPolygon->isChecked()) {

		if (e->button() == Qt::LeftButton) {

			if (w->hasObject() && !w->getDrawPolygonActivated()) {
				return;
			}

			QVector<QPoint> pts = w->getObjectPoints();
			pts.push_back(e->pos());

			w->setObjectPoints(pts);
			w->setDrawPolygonActivated(true);

			w->clear();

			for (int i = 0; i < pts.size() - 1; i++) {
				w->drawLine(
					pts[i],
					pts[i + 1],
					globalColor,
					ui->comboBoxLineAlg->currentIndex()
				);
			}

			return;
		}

		if (e->button() == Qt::RightButton && w->getDrawPolygonActivated()) {
			QVector<QPoint> pts = w->getObjectPoints();

			if (pts.size() >= 3) {
				if (pts.size() == 3) {
					w->setCurrentObjectType(ViewerWidget::ObjectTriangle);
				}
				else {
					w->setCurrentObjectType(ViewerWidget::ObjectPolygon);
				}

				w->setDrawPolygonActivated(false);
				w->setShapeFilled(false);
				w->setFillMode(ViewerWidget::FillNone);

				w->redrawScene(globalColor, ui->comboBoxLineAlg->currentIndex());
			}
			else {
				w->setDrawPolygonActivated(false);
				w->setObjectPoints(QVector<QPoint>());
				w->clear();
			}

			return;
		}
	}

	if (e->button() == Qt::LeftButton) {
		if (w->hasObject() && w->isPointNearObject(e->pos())) {
			w->setDragging(true);
			w->setLastMousePos(e->pos());
			return;
		}
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

	if (w->getDragging()) {
		QPoint currentPos = e->pos();
		QPoint lastPos = w->getLastMousePos();

		int dx = currentPos.x() - lastPos.x();
		int dy = currentPos.y() - lastPos.y();

		if (dx != 0 || dy != 0) {
			w->translateObject(dx, dy);
			w->setLastMousePos(currentPos);
			w->redrawScene(globalColor, ui->comboBoxLineAlg->currentIndex());
		}
		return;
	}

	if (ui->toolButtonDrawLine->isChecked() && w->getDrawLineActivated()) {
		w->clear();
		w->setPixel(w->getDrawLineBegin().x(), w->getDrawLineBegin().y(), globalColor);
		w->drawLine(w->getDrawLineBegin(), e->pos(), globalColor,
			ui->comboBoxLineAlg->currentIndex());
		w->update();
		return;
	}

	if (ui->toolButtonDrawCircle->isChecked() && w->getDrawCircleActivated()) {
		QPoint center = w->getDrawCircleCenter();
		QPoint p = e->pos();

		int dx = p.x() - center.x();
		int dy = p.y() - center.y();
		int radius = static_cast<int>(std::round(std::sqrt(dx * dx + dy * dy)));

		w->clear();
		w->setPixel(center.x(), center.y(), globalColor);
		w->drawCircle(center, radius, globalColor);
		w->update();
		return;
	}

	if (ui->toolButtonDrawPolygon->isChecked() && w->getDrawPolygonActivated()) {
		QVector<QPoint> pts = w->getObjectPoints();

		w->clear();

		for (int i = 0; i < pts.size(); i++) {
			w->setPixel(pts[i].x(), pts[i].y(), globalColor);
		}

	
		for (int i = 0; i < pts.size() - 1; i++) {
			w->drawLine(pts[i], pts[i + 1], globalColor,
				ui->comboBoxLineAlg->currentIndex());
		}


		if (!pts.isEmpty()) {
			w->drawLine(pts.last(), e->pos(), globalColor,
				ui->comboBoxLineAlg->currentIndex());
		}

		w->update();
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
	QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);

	if (!w->hasObject()) {
		return;
	}

	QPoint delta = wheelEvent->angleDelta();

	if (delta.y() > 0) {
		w->scaleObject(1.25, 1.25);
		ui->doubleSpinBoxScaleX->setValue(1.25);
		ui->doubleSpinBoxScaleY->setValue(1.25);
		w->redrawScene(globalColor, ui->comboBoxLineAlg->currentIndex());
	}
	else if (delta.y() < 0) {
		w->scaleObject(0.75, 0.75);
		ui->doubleSpinBoxScaleX->setValue(0.75);
		ui->doubleSpinBoxScaleY->setValue(0.75);
		w->redrawScene(globalColor, ui->comboBoxLineAlg->currentIndex());
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

void ImageViewer::saveToVTK(QString filename)
{
	QFileInfo fi(filename);
	QString extension = fi.completeSuffix();
	return;
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
void ImageViewer::on_actionClear_triggered()
{
	vW->clearAll();
}

void ImageViewer::on_actionExit_triggered()
{
	this->close();
}

void ImageViewer::on_toolButtonDrawLine_clicked()
{
	if (ui->toolButtonDrawLine->isChecked()) {
		ui->toolButtonDrawCircle->setChecked(false);
		ui->toolButtonDrawPolygon->setChecked(false);
		ui->comboBoxLineAlg->setEnabled(true);
		vW->setDrawCircleActivated(false);
		vW->setDrawPolygonActivated(false);
	}
	else {
		ui->comboBoxLineAlg->setEnabled(false);
	}
}

void ImageViewer::on_toolButtonDrawCircle_clicked()
{
	if (ui->toolButtonDrawCircle->isChecked()) {
		ui->toolButtonDrawLine->setChecked(false);
		ui->toolButtonDrawPolygon->setChecked(false);
		ui->comboBoxLineAlg->setEnabled(true);
		vW->setDrawLineActivated(false);
		vW->setDrawPolygonActivated(false);
	}
	else {
		ui->comboBoxLineAlg->setEnabled(false);
	}
}

void ImageViewer::on_toolButtonDrawPolygon_clicked()
{
	if (ui->toolButtonDrawPolygon->isChecked()) {
		ui->toolButtonDrawLine->setChecked(false);
		ui->toolButtonDrawCircle->setChecked(false);
		ui->comboBoxLineAlg->setEnabled(true);

		vW->setDrawLineActivated(false);
		vW->setDrawCircleActivated(false);
		vW->setDrawPolygonActivated(false);
	}
	else {
		ui->comboBoxLineAlg->setEnabled(false);
	}
}

void ImageViewer::on_toolButtonRotate_clicked()
{
	if (!vW->hasObject()) {
		return;
	}

	int angle = ui->doubleSpinBoxRotate->value();

	vW->rotateObject(angle);
	vW->redrawScene(globalColor, ui->comboBoxLineAlg->currentIndex());
}

void ImageViewer::on_toolButtonShearX_clicked()
{
	if (!vW->hasObject()) {
		return;
	}

	double shx = ui->doubleSpinBoxShearX->value();

	vW->shearX(shx);
	vW->redrawScene(globalColor, ui->comboBoxLineAlg->currentIndex());
}

void ImageViewer::on_toolButtonScale_clicked()
{
	if (!vW->hasObject()) {
		return;
	}

	double kx = ui->doubleSpinBoxScaleX->value();
	double ky = ui->doubleSpinBoxScaleY->value();

	vW->scaleObject(kx, ky);
	vW->redrawScene(globalColor, ui->comboBoxLineAlg->currentIndex());
}
void ImageViewer::on_toolButtonSymmetry_clicked()
{
	if (!vW->hasObject()) {
		return;
	}

	vW->symmetry();
	vW->redrawScene(globalColor, ui->comboBoxLineAlg->currentIndex());
}

void ImageViewer::on_toolButtonFillShape_clicked()
{
	if (!vW->hasObject()) {
		return;
	}

	int index = ui->comboBoxFillMode->currentIndex();

	if (index == 0) {
		vW->setFillMode(ViewerWidget::FillSolidColor);
	}
	else if (index == 1) {
		if (!vW->isTriangle()) {
			return;
		}

		vW->setFillMode(ViewerWidget::FillBarycentric);
	}
	else if (index == 2) {
		if (!vW->isTriangle()) {
			return;
		}

		vW->setFillMode(ViewerWidget::FillNearestNeighbour);
	}

	vW->setShapeFilled(true);
	vW->setFillColor(globalColor);
	vW->redrawScene(globalColor, ui->comboBoxLineAlg->currentIndex());
}

void ImageViewer::on_pushButtonT1_clicked()
{
	QColor newColor = QColorDialog::getColor(vW->getTriangleColor0(), this);

	if (newColor.isValid()) {
		QString style_sheet = QString("background-color: %1;")
			.arg(newColor.name(QColor::HexRgb));

		ui->pushButtonT1->setStyleSheet(style_sheet);
		vW->setTriangleColor0(newColor);
	}
}

void ImageViewer::on_pushButtonT2_clicked()
{
	QColor newColor = QColorDialog::getColor(vW->getTriangleColor1(), this);

	if (newColor.isValid()) {
		QString style_sheet = QString("background-color: %1;")
			.arg(newColor.name(QColor::HexRgb));

		ui->pushButtonT2->setStyleSheet(style_sheet);
		vW->setTriangleColor1(newColor);
	}
}

void ImageViewer::on_pushButtonT3_clicked()
{
	QColor newColor = QColorDialog::getColor(vW->getTriangleColor2(), this);

	if (newColor.isValid()) {
		QString style_sheet = QString("background-color: %1;")
			.arg(newColor.name(QColor::HexRgb));

		ui->pushButtonT3->setStyleSheet(style_sheet);
		vW->setTriangleColor2(newColor);
	}
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

