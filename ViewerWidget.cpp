#include "ViewerWidget.h"

namespace
{
	QVector<QPoint> g_triangleOriginalPts;
}

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

// Image functions
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
	if (img == nullptr) return true;
	if (img->size() == QSize(0, 0)) return true;
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

	setPixel(
		x, y,
		static_cast<int>(255 * valR + 0.5),
		static_cast<int>(255 * valG + 0.5),
		static_cast<int>(255 * valB + 0.5),
		static_cast<int>(255 * valA + 0.5)
	);
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

// Draw functions
void ViewerWidget::drawLine(QPoint start, QPoint end, QColor color, int algType)
{
	if (!img || !data) return;

	if (algType == 0) {
		drawLineDDA(start, end, color);
	}
	else {
		drawLineBresenham(start, end, color);
	}
}

void ViewerWidget::drawCircle(QPoint center, int radius, QColor color)
{
	if (!img || !data || radius < 0) return;

	drawCircleBresenham(center, radius, color);
	update();
}

void ViewerWidget::drawCircleBresenham(QPoint center, int radius, QColor color)
{
	int xc = center.x();
	int yc = center.y();

	int x = 0;
	int y = radius;
	int p = 1 - radius;

	while (x <= y)
	{
		setPixel(xc + x, yc + y, color);
		setPixel(xc - x, yc + y, color);
		setPixel(xc + x, yc - y, color);
		setPixel(xc - x, yc - y, color);

		setPixel(xc + y, yc + x, color);
		setPixel(xc - y, yc + x, color);
		setPixel(xc + y, yc - x, color);
		setPixel(xc - y, yc - x, color);

		if (p < 0) {
			p = p + 2 * x + 3;
		}
		else {
			p = p + 2 * x - 2 * y + 5;
			y = y - 1;
		}

		x = x + 1;
	}
}

void ViewerWidget::clear()
{
	if (!img) return;
	img->fill(Qt::white);
	update();
}

void ViewerWidget::clearAll()
{
	if (!img) return;

	img->fill(Qt::white);

	currentObjectType = ObjectNone;
	objectPoints.clear();

	drawLineActivated = false;
	drawCircleActivated = false;
	drawPolygonActivated = false;
	dragging = false;

	currentFillMode = FillNone;
	shapeFilled = false;

	update();
}

void ViewerWidget::drawLineDDA(QPoint start, QPoint end, QColor color)
{
	int x1 = start.x();
	int y1 = start.y();
	int x2 = end.x();
	int y2 = end.y();

	int dx = x2 - x1;
	int dy = y2 - y1;

	int steps = std::max(std::abs(dx), std::abs(dy));
	if (steps == 0) {
		setPixel(x1, y1, color);
		return;
	}

	double xInc = static_cast<double>(dx) / steps;
	double yInc = static_cast<double>(dy) / steps;

	double x = x1;
	double y = y1;

	for (int i = 0; i <= steps; i++) {
		setPixel(
			static_cast<int>(std::round(x)),
			static_cast<int>(std::round(y)),
			color
		);
		x += xInc;
		y += yInc;
	}
}

void ViewerWidget::drawLineBresenham(QPoint start, QPoint end, QColor color)
{
	int x1 = start.x();
	int y1 = start.y();
	int x2 = end.x();
	int y2 = end.y();

	int dx = std::abs(x2 - x1);
	int dy = std::abs(y2 - y1);

	int sx = (x1 < x2) ? 1 : -1;
	int sy = (y1 < y2) ? 1 : -1;

	int err = dx - dy;

	while (true) {
		setPixel(x1, y1, color);

		if (x1 == x2 && y1 == y2)
			break;

		int e2 = 2 * err;

		if (e2 > -dy) {
			err -= dy;
			x1 += sx;
		}

		if (e2 < dx) {
			err += dx;
			y1 += sy;
		}
	}
}


bool ViewerWidget::hasObject() const
{
	return currentObjectType != ObjectNone && !objectPoints.isEmpty();
}

QPoint ViewerWidget::getReferencePoint() const
{
	if (objectPoints.isEmpty()) {
		return QPoint(0, 0);
	}
	return objectPoints[0];
}

void ViewerWidget::drawPolygon(const QVector<QPoint>& pts, QColor color, int algType)
{
	if (pts.size() < 2) return;

	for (int i = 0; i < pts.size() - 1; i++) {
		drawLine(pts[i], pts[i + 1], color, algType);
	}

	drawLine(pts.last(), pts.first(), color, algType);
}

void ViewerWidget::drawCurrentObject(QColor color, int algType)
{
	if (!hasObject()) return;

	if (currentObjectType == ObjectLine) {
		if (objectPoints.size() >= 2) {
			drawLine(objectPoints[0], objectPoints[1], color, algType);
		}
	}
	else if (currentObjectType == ObjectPolygon || currentObjectType == ObjectTriangle) {
		if (objectPoints.size() >= 3) {
			drawPolygon(objectPoints, color, algType);
		}
	}
}

void ViewerWidget::redrawScene(QColor color, int algType)
{
	if (!img) return;

	img->fill(Qt::white);

	if (!hasObject()) {
		update();
		return;
	}

	if (!clippingEnabled) {
		if (shapeFilled) {
			fillCurrentShape();
		}
		drawCurrentObject(color, algType);
		update();
		return;
	}

	if (currentObjectType == ObjectLine) {
		if (objectPoints.size() >= 2) {
			QPoint c1, c2;
			if (clipLineCyrusBeck(objectPoints[0], objectPoints[1], c1, c2)) {
				drawLine(c1, c2, color, algType);
			}
		}
	}
	else if (currentObjectType == ObjectPolygon) {
		if (objectPoints.size() >= 3) {
			QVector<QPoint> clipped = clipPolygonSutherlandHodgman(objectPoints);
			if (clipped.size() >= 3) {
				if (shapeFilled && currentFillMode == FillSolidColor) {
					fillPolygonScanLine(clipped, fillColor);
				}
				drawPolygon(clipped, color, algType);
			}
		}
	}
	else if (currentObjectType == ObjectTriangle) {
		if (objectPoints.size() >= 3) {
			QVector<QPoint> clipped = clipPolygonSutherlandHodgman(objectPoints);
			if (clipped.size() >= 3) {
				if (shapeFilled) {
					if (currentFillMode == FillSolidColor || clipped.size() != 3) {
						fillPolygonScanLine(clipped, fillColor);
					}
					else if (clipped.size() == 3) {
						if (currentFillMode == FillNearestNeighbour ||
							currentFillMode == FillBarycentric) {
							fillTriangleInterpolated(clipped, currentFillMode);
						}
					}
				}
				drawPolygon(clipped, color, algType);
			}
		}
	}

	update();
}

void ViewerWidget::translateObject(int dx, int dy)
{
	if (!hasObject()) return;

	for (int i = 0; i < objectPoints.size(); i++) {
		objectPoints[i].setX(objectPoints[i].x() + dx);
		objectPoints[i].setY(objectPoints[i].y() + dy);
	}
}

void ViewerWidget::rotateObject(double angleDeg)
{
	if (!hasObject()) return;

	QPoint ref = getReferencePoint();
	double angleRad = angleDeg * M_PI / 180.0;

	double c = std::cos(angleRad);
	double s = std::sin(angleRad);

	for (int i = 0; i < objectPoints.size(); i++) {
		int x = objectPoints[i].x();
		int y = objectPoints[i].y();

		double xr = ref.x() + (x - ref.x()) * c - (y - ref.y()) * s;
		double yr = ref.y() + (x - ref.x()) * s + (y - ref.y()) * c;

		objectPoints[i] = QPoint(
			static_cast<int>(std::round(xr)),
			static_cast<int>(std::round(yr))
		);
	}
}

void ViewerWidget::scaleObject(double kx, double ky)
{
	if (!hasObject()) return;

	QPoint ref = getReferencePoint();

	for (int i = 0; i < objectPoints.size(); i++) {
		double xr = ref.x() + (objectPoints[i].x() - ref.x()) * kx;
		double yr = ref.y() + (objectPoints[i].y() - ref.y()) * ky;

		objectPoints[i] = QPoint(
			static_cast<int>(std::round(xr)),
			static_cast<int>(std::round(yr))
		);
	}
}

void ViewerWidget::shearX(double shx)
{
	if (!hasObject()) return;

	QPoint ref = getReferencePoint();

	for (int i = 0; i < objectPoints.size(); i++) {
		double xLocal = objectPoints[i].x() - ref.x();
		double yLocal = objectPoints[i].y() - ref.y();

		double xr = xLocal + shx * yLocal;
		double yr = yLocal;

		objectPoints[i] = QPoint(
			static_cast<int>(std::round(ref.x() + xr)),
			static_cast<int>(std::round(ref.y() + yr))
		);
	}
}

void ViewerWidget::symmetry()
{
	if (!hasObject()) return;

	if (currentObjectType == ObjectLine) {
		int x0 = objectPoints[0].x();

		for (int i = 0; i < objectPoints.size(); i++) {
			int x = objectPoints[i].x();
			int y = objectPoints[i].y();
			objectPoints[i] = QPoint(2 * x0 - x, y);
		}
	}
	else if (currentObjectType == ObjectPolygon || currentObjectType == ObjectTriangle) {
		if (objectPoints.size() < 2) return;

		QPoint A = objectPoints[0];
		QPoint B = objectPoints[1];

		double u = B.x() - A.x();
		double v = B.y() - A.y();

		double a = v;
		double b = -u;
		double c = -a * A.x() - b * A.y();

		double denom = a * a + b * b;
		if (denom == 0.0) return;

		for (int i = 0; i < objectPoints.size(); i++) {
			double x = objectPoints[i].x();
			double y = objectPoints[i].y();

			double d = (a * x + b * y + c) / denom;

			double xr = x - 2.0 * a * d;
			double yr = y - 2.0 * b * d;

			objectPoints[i] = QPoint(
				static_cast<int>(std::round(xr)),
				static_cast<int>(std::round(yr))
			);
		}
	}
}

double ViewerWidget::distancePointToSegment(QPoint p, QPoint a, QPoint b) const
{
	double px = p.x();
	double py = p.y();
	double ax = a.x();
	double ay = a.y();
	double bx = b.x();
	double by = b.y();

	double dx = bx - ax;
	double dy = by - ay;

	if (dx == 0.0 && dy == 0.0) {
		double distX = px - ax;
		double distY = py - ay;
		return std::sqrt(distX * distX + distY * distY);
	}

	double t = ((px - ax) * dx + (py - ay) * dy) / (dx * dx + dy * dy);

	if (t < 0.0) t = 0.0;
	if (t > 1.0) t = 1.0;

	double cx = ax + t * dx;
	double cy = ay + t * dy;

	double distX = px - cx;
	double distY = py - cy;

	return std::sqrt(distX * distX + distY * distY);
}

bool ViewerWidget::isInsideClipEdge(const QPoint& p, int edge, int boundary) const
{
	// 0 = left, 1 = right, 2 = top, 3 = bottom
	if (edge == 0) return p.x() >= boundary;
	if (edge == 1) return p.x() <= boundary;
	if (edge == 2) return p.y() >= boundary;
	if (edge == 3) return p.y() <= boundary;
	return false;
}

QPoint ViewerWidget::intersectWithClipEdge(const QPoint& s, const QPoint& e, int edge, int boundary) const
{
	double x1 = s.x();
	double y1 = s.y();
	double x2 = e.x();
	double y2 = e.y();

	double dx = x2 - x1;
	double dy = y2 - y1;

	if (edge == 0 || edge == 1) {
		// x = boundary
		if (std::abs(dx) < 1e-9) {
			return QPoint(boundary, static_cast<int>(std::round(y1)));
		}

		double t = (boundary - x1) / dx;
		double y = y1 + t * dy;
		return QPoint(boundary, static_cast<int>(std::round(y)));
	}
	else {
		// y = boundary
		if (std::abs(dy) < 1e-9) {
			return QPoint(static_cast<int>(std::round(x1)), boundary);
		}

		double t = (boundary - y1) / dy;
		double x = x1 + t * dx;
		return QPoint(static_cast<int>(std::round(x)), boundary);
	}
}

bool ViewerWidget::isPointNearObject(QPoint p) const
{
	if (!hasObject()) return false;

	const double eps = 8.0;

	if (currentObjectType == ObjectLine) {
		if (objectPoints.size() < 2) return false;
		return distancePointToSegment(p, objectPoints[0], objectPoints[1]) <= eps;
	}

	if (currentObjectType == ObjectPolygon || currentObjectType == ObjectTriangle) {
		if (objectPoints.size() < 2) return false;

		for (int i = 0; i < objectPoints.size() - 1; i++) {
			if (distancePointToSegment(p, objectPoints[i], objectPoints[i + 1]) <= eps) {
				return true;
			}
		}

		if (distancePointToSegment(p, objectPoints.last(), objectPoints.first()) <= eps) {
			return true;
		}
	}

	return false;
}

bool ViewerWidget::clipLineCyrusBeck(QPoint p1, QPoint p2, QPoint& out1, QPoint& out2) const
{
	if (!img) return false;

	// прямоугольник отсечения в направлении обхода по часовой стрелке
	QVector<QPointF> clipPolygon;
	clipPolygon.push_back(QPointF(0, 0));
	clipPolygon.push_back(QPointF(0, img->height() - 1));
	clipPolygon.push_back(QPointF(img->width() - 1, img->height() - 1));
	clipPolygon.push_back(QPointF(img->width() - 1, 0));

	QPointF P0(p1);
	QPointF P1(p2);
	QPointF D = P1 - P0;

	double tE = 0.0;
	double tL = 1.0;

	for (int i = 0; i < clipPolygon.size(); i++) {
		QPointF E = clipPolygon[i];
		QPointF E2 = clipPolygon[(i + 1) % clipPolygon.size()];
		QPointF edge = E2 - E;
		QPointF n(edge.y(), -edge.x());

		double numerator = QPointF::dotProduct(n, E - P0);
		double denominator = QPointF::dotProduct(n, D);

		if (std::abs(denominator) < 1e-9) {
			// rovnobežná úsečka
			if (numerator < 0) {
				return false;
			}
		}
		else {
			double t = numerator / denominator;

			if (denominator > 0) {

				if (t > tE) tE = t;
			}
			else {
			
				if (t < tL) tL = t;
			}

			if (tE > tL) {
				return false;
			}
		}
	}

	QPointF C0 = P0 + D * tE;
	QPointF C1 = P0 + D * tL;

	out1 = QPoint(static_cast<int>(std::round(C0.x())),
		static_cast<int>(std::round(C0.y())));
	out2 = QPoint(static_cast<int>(std::round(C1.x())),
		static_cast<int>(std::round(C1.y())));

	return true;
}

QVector<QPoint> ViewerWidget::clipPolygonSutherlandHodgman(const QVector<QPoint>& polygon) const
{
	if (!img || polygon.size() < 3) {
		return QVector<QPoint>();
	}

	QVector<QPoint> output = polygon;

	int left = 0;
	int right = img->width() - 1;
	int top = 0;
	int bottom = img->height() - 1;

	for (int edge = 0; edge < 4; edge++) {
		QVector<QPoint> input = output;
		output.clear();

		if (input.isEmpty()) {
			break;
		}

		int boundary = 0;
		if (edge == 0) boundary = left;
		if (edge == 1) boundary = right;
		if (edge == 2) boundary = top;
		if (edge == 3) boundary = bottom;

		QPoint S = input.last();

		for (int i = 0; i < input.size(); i++) {
			QPoint E = input[i];

			bool E_inside = isInsideClipEdge(E, edge, boundary);
			bool S_inside = isInsideClipEdge(S, edge, boundary);

			if (E_inside) {
				if (!S_inside) {
					output.push_back(intersectWithClipEdge(S, E, edge, boundary));
				}
				output.push_back(E);
			}
			else if (S_inside) {
				output.push_back(intersectWithClipEdge(S, E, edge, boundary));
			}

			S = E;
		}
	}

	return output;
}

void ViewerWidget::fillCurrentShape()
{
	if (!shapeFilled) {
		return;
	}

	// обычный полигон
	if (currentObjectType == ObjectPolygon) {
		fillPolygonScanLine(objectPoints, fillColor);
		return;
	}

	// треугольник
	if (currentObjectType == ObjectTriangle) {

		if (currentFillMode == FillSolidColor) {
			fillTriangleSolid(objectPoints, fillColor);
		}
		else if (currentFillMode == FillNearestNeighbour ||
			currentFillMode == FillBarycentric) {
			fillTriangleInterpolated(objectPoints, currentFillMode);
		}

		return;
	}
}

QVector<QVector<ViewerWidget::ScanLineEdge>> ViewerWidget::createEdgeTable(const QVector<QPoint>& pts, int& yMin, int& yMax) const
{
	QVector<QVector<ScanLineEdge>> edgeTable;

	if (pts.size() < 3) {
		yMin = 0;
		yMax = -1;
		return edgeTable;
	}

	yMin = pts[0].y();
	yMax = pts[0].y();

	for (int i = 1; i < pts.size(); i++) {
		if (pts[i].y() < yMin) yMin = pts[i].y();
		if (pts[i].y() > yMax) yMax = pts[i].y();
	}

	edgeTable.resize(yMax - yMin + 1);

	for (int i = 0; i < pts.size(); i++) {
		QPoint p1 = pts[i];
		QPoint p2 = pts[(i + 1) % pts.size()];

		if (p1.y() == p2.y()) {
			continue; // горизонтальный край не добавляем
		}

		QPoint top = p1;
		QPoint bottom = p2;

		if (top.y() > bottom.y()) {
			std::swap(top, bottom);
		}

		ScanLineEdge edge;
		edge.yMax = bottom.y() - 1; // сокращение на 1 px снизу 
		edge.x = top.x();
		edge.invM = static_cast<double>(bottom.x() - top.x()) /
			static_cast<double>(bottom.y() - top.y());

		int row = top.y() - yMin;
		if (row >= 0 && row < edgeTable.size()) {
			edgeTable[row].push_back(edge);
		}
	}

	return edgeTable;
}

void ViewerWidget::fillPolygonScanLine(const QVector<QPoint>& pts, const QColor& color)
{
	if (pts.size() < 3) return;

	int yMin = 0;
	int yMax = 0;
	QVector<QVector<ScanLineEdge>> edgeTable = createEdgeTable(pts, yMin, yMax);

	if (edgeTable.isEmpty()) return;

	QVector<ScanLineEdge> activeEdges;

	for (int y = yMin; y <= yMax; y++) {
		int row = y - yMin;

		if (row >= 0 && row < edgeTable.size()) {
			for (int i = 0; i < edgeTable[row].size(); i++) {
				activeEdges.push_back(edgeTable[row][i]);
			}
		}

		for (int i = activeEdges.size() - 1; i >= 0; i--) {
			if (y > activeEdges[i].yMax) {
				activeEdges.remove(i);
			}
		}

		std::sort(activeEdges.begin(), activeEdges.end(),
			[](const ScanLineEdge& a, const ScanLineEdge& b) {
				return a.x < b.x;
			});

		for (int i = 0; i + 1 < activeEdges.size(); i += 2) {
			int xStart = static_cast<int>(std::ceil(activeEdges[i].x));
			int xEnd = static_cast<int>(std::floor(activeEdges[i + 1].x));

			for (int x = xStart; x <= xEnd; x++) {
				setPixel(x, y, color);
			}
		}

		for (int i = 0; i < activeEdges.size(); i++) {
			activeEdges[i].x += activeEdges[i].invM;
		}
	}
}

void ViewerWidget::sortTriangleVertices(QVector<QPoint>& pts) const
{
	std::sort(pts.begin(), pts.end(),
		[](const QPoint& a, const QPoint& b) {
			if (a.y() != b.y()) return a.y() < b.y();
			return a.x() < b.x();
		});
}

void ViewerWidget::fillTriangleSolid(const QVector<QPoint>& pts, const QColor& color)
{
	if (pts.size() != 3) return;

	g_triangleOriginalPts = pts;
	currentFillMode = FillSolidColor;
	fillColor = color;

	QVector<QPoint> sorted = pts;
	sortTriangleVertices(sorted);

	QPoint v0 = sorted[0];
	QPoint v1 = sorted[1];
	QPoint v2 = sorted[2];

	if (v0.y() == v1.y()) {
		fillTopFlatTriangle(v0, v1, v2);
		return;
	}

	if (v1.y() == v2.y()) {
		fillBottomFlatTriangle(v0, v1, v2);
		return;
	}

	double xSplit = v0.x() +
		(static_cast<double>(v1.y() - v0.y()) * (v2.x() - v0.x())) /
		static_cast<double>(v2.y() - v0.y());

	QPoint p(static_cast<int>(std::round(xSplit)), v1.y());

	if (v1.x() < p.x()) {
		fillBottomFlatTriangle(v0, v1, p);
		fillTopFlatTriangle(v1, p, v2);
	}
	else {
		fillBottomFlatTriangle(v0, p, v1);
		fillTopFlatTriangle(p, v1, v2);
	}
}

void ViewerWidget::fillTriangleInterpolated(const QVector<QPoint>& pts, FillMode mode)
{
	if (pts.size() != 3) return;
	if (mode != FillNearestNeighbour && mode != FillBarycentric) return;

	g_triangleOriginalPts = pts;
	currentFillMode = mode;

	QVector<QPoint> sorted = pts;
	sortTriangleVertices(sorted);

	QPoint v0 = sorted[0];
	QPoint v1 = sorted[1];
	QPoint v2 = sorted[2];

	if (v0.y() == v1.y()) {
		fillTopFlatTriangle(v0, v1, v2);
		return;
	}

	if (v1.y() == v2.y()) {
		fillBottomFlatTriangle(v0, v1, v2);
		return;
	}

	double xSplit = v0.x() +
		(static_cast<double>(v1.y() - v0.y()) * (v2.x() - v0.x())) /
		static_cast<double>(v2.y() - v0.y());

	QPoint p(static_cast<int>(std::round(xSplit)), v1.y());

	if (v1.x() < p.x()) {
		fillBottomFlatTriangle(v0, v1, p);
		fillTopFlatTriangle(v1, p, v2);
	}
	else {
		fillBottomFlatTriangle(v0, p, v1);
		fillTopFlatTriangle(p, v1, v2);
	}
}

void ViewerWidget::fillBottomFlatTriangle(const QPoint& v0, const QPoint& v1, const QPoint& v2)
{
	if (v1.y() == v0.y() || v2.y() == v0.y()) return;

	double invM1 = static_cast<double>(v1.x() - v0.x()) /
		static_cast<double>(v1.y() - v0.y());
	double invM2 = static_cast<double>(v2.x() - v0.x()) /
		static_cast<double>(v2.y() - v0.y());

	double x1 = v0.x();
	double x2 = v0.x();

	for (int y = v0.y(); y < v1.y(); y++) {
		drawTriangleScanLine(y, x1, x2, g_triangleOriginalPts, currentFillMode);
		x1 += invM1;
		x2 += invM2;
	}
}

void ViewerWidget::fillTopFlatTriangle(const QPoint& v0, const QPoint& v1, const QPoint& v2)
{
	if (v2.y() == v0.y() || v2.y() == v1.y()) return;

	double invM1 = static_cast<double>(v2.x() - v0.x()) /
		static_cast<double>(v2.y() - v0.y());
	double invM2 = static_cast<double>(v2.x() - v1.x()) /
		static_cast<double>(v2.y() - v1.y());

	double x1 = v0.x();
	double x2 = v1.x();

	for (int y = v0.y(); y < v2.y(); y++) {
		drawTriangleScanLine(y, x1, x2, g_triangleOriginalPts, currentFillMode);
		x1 += invM1;
		x2 += invM2;
	}
}

void ViewerWidget::drawTriangleScanLine(int y, double x1, double x2,
	const QVector<QPoint>& pts,
	FillMode mode)
{
	int xStart = static_cast<int>(std::ceil(std::min(x1, x2)));
	int xEnd = static_cast<int>(std::floor(std::max(x1, x2)));

	for (int x = xStart; x <= xEnd; x++) {
		QColor color = fillColor;

		if (mode == FillNearestNeighbour) {
			color = getNearestNeighbourColor(
				QPoint(x, y), pts,
				triangleColor0, triangleColor1, triangleColor2
			);
		}
		else if (mode == FillBarycentric) {
			color = getBarycentricColor(
				QPoint(x, y), pts,
				triangleColor0, triangleColor1, triangleColor2
			);
		}

		setPixel(x, y, color);
	}
}

QColor ViewerWidget::getNearestNeighbourColor(const QPoint& p,const QVector<QPoint>& pts,const QColor& c0,const QColor& c1,const QColor& c2) const
{
	if (pts.size() != 3) return c0;

	double d0x = p.x() - pts[0].x();
	double d0y = p.y() - pts[0].y();
	double d1x = p.x() - pts[1].x();
	double d1y = p.y() - pts[1].y();
	double d2x = p.x() - pts[2].x();
	double d2y = p.y() - pts[2].y();

	double d0 = d0x * d0x + d0y * d0y;
	double d1 = d1x * d1x + d1y * d1y;
	double d2 = d2x * d2x + d2y * d2y;

	if (d0 <= d1 && d0 <= d2) return c0;
	if (d1 <= d0 && d1 <= d2) return c1;
	return c2;
}

double ViewerWidget::triangleArea2(const QPoint& a, const QPoint& b, const QPoint& c) const
{
	return static_cast<double>(b.x() - a.x()) * static_cast<double>(c.y() - a.y()) -
		static_cast<double>(b.y() - a.y()) * static_cast<double>(c.x() - a.x());
}

void ViewerWidget::clear3Ddata()
{
	half_edges.clear();
	faces.clear();
	vertices.clear();
}

void ViewerWidget::halfEdgeMesh(const QVector<Point3D>& points, const QVector<TriangleIndices>& triangles)
{
	clear3Ddata();

	for (int i = 0; i < points.size(); i++)
	{
		Vertex* v = new Vertex;
		v->x = points[i].x;
		v->y = points[i].y;
		v->z = points[i].z;
		v->edge = nullptr;

		vertices.push_back(v);
	}

	for (int i = 0; i < triangles.size(); i++)
	{
		TriangleIndices t = triangles[i];

		Face* f = new Face;
		f->edge = nullptr;

		H_edge* h1 = new H_edge;
		H_edge* h2 = new H_edge;
		H_edge* h3 = new H_edge;

		h1->vert_origin = vertices[t.v1];
		h2->vert_origin = vertices[t.v2];
		h3->vert_origin = vertices[t.v3];

		h1->face = f;
		h2->face = f;
		h3->face = f;

		h1->edge_next = h2;
		h2->edge_next = h3;
		h3->edge_next = h1;

		h1->edge_prev = h3;
		h2->edge_prev = h1;
		h3->edge_prev = h2;

		h1->pair = nullptr;
		h2->pair = nullptr;
		h3->pair = nullptr;

		f->edge = h1;

		if (vertices[t.v1]->edge == nullptr)
			vertices[t.v1]->edge = h1;

		if (vertices[t.v2]->edge == nullptr)
			vertices[t.v2]->edge = h2;

		if (vertices[t.v3]->edge == nullptr)
			vertices[t.v3]->edge = h3;

		faces.push_back(f);
		half_edges.push_back(h1);
		half_edges.push_back(h2);
		half_edges.push_back(h3);
	}

	for (int i = 0; i < half_edges.size(); i++)
	{
		H_edge* h1 = half_edges[i];

		Vertex* start1 = h1->vert_origin;
		Vertex* end1 = h1->edge_next->vert_origin;

		for (int j = 0; j < half_edges.size(); j++)
		{
			if (i == j)
			{
				continue;
			}

			H_edge* h2 = half_edges[j];

			Vertex* start2 = h2->vert_origin;
			Vertex* end2 = h2->edge_next->vert_origin;

			if (start1 == end2 && end1 == start2)
			{
				h1->pair = h2;
				break;
			}
		}
	}
}

void ViewerWidget::createCube(double side)
{
	QVector<Point3D> points;
	QVector<TriangleIndices> triangles;

	double h = side / 2.0;

	// 8 vertices of cube
	points.push_back({ -h, -h, -h }); // 0
	points.push_back({ h, -h, -h }); // 1
	points.push_back({ h,  h, -h }); // 2
	points.push_back({ -h,  h, -h }); // 3

	points.push_back({ -h, -h,  h }); // 4
	points.push_back({ h, -h,  h }); // 5
	points.push_back({ h,  h,  h }); // 6
	points.push_back({ -h,  h,  h }); // 7

	// bottom face
	triangles.push_back({ 0, 2, 1 });
	triangles.push_back({ 0, 3, 2 });

	// top face
	triangles.push_back({ 4, 5, 6 });
	triangles.push_back({ 4, 6, 7 });

	// front face
	triangles.push_back({ 0, 1, 5 });
	triangles.push_back({ 0, 5, 4 });

	// back face
	triangles.push_back({ 3, 6, 2 });
	triangles.push_back({ 3, 7, 6 });

	// left face
	triangles.push_back({ 0, 4, 7 });
	triangles.push_back({ 0, 7, 3 });

	// right face
	triangles.push_back({ 1, 2, 6 });
	triangles.push_back({ 1, 6, 5 });

	halfEdgeMesh(points, triangles);
}

QColor ViewerWidget::getBarycentricColor(const QPoint& p,const QVector<QPoint>& pts,const QColor& c0,const QColor& c1,const QColor& c2) const
{
	if (pts.size() != 3) return c0;

	double denom = triangleArea2(pts[0], pts[1], pts[2]);
	if (std::abs(denom) < 1e-9) return c0;

	double lambda0 = triangleArea2(p, pts[1], pts[2]) / denom;
	double lambda1 = triangleArea2(pts[0], p, pts[2]) / denom;
	double lambda2 = 1.0 - lambda0 - lambda1;

	int r = static_cast<int>(std::round(
		lambda0 * c0.red() +
		lambda1 * c1.red() +
		lambda2 * c2.red()
	));

	int g = static_cast<int>(std::round(
		lambda0 * c0.green() +
		lambda1 * c1.green() +
		lambda2 * c2.green()
	));

	int b = static_cast<int>(std::round(
		lambda0 * c0.blue() +
		lambda1 * c1.blue() +
		lambda2 * c2.blue()
	));

	r = std::max(0, std::min(255, r));
	g = std::max(0, std::min(255, g));
	b = std::max(0, std::min(255, b));

	return QColor(r, g, b);
}

// Slots
void ViewerWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	if (!img || img->isNull()) return;

	QRect area = event->rect();
	painter.drawImage(area, *img, area);
}