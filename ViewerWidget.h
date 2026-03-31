#pragma once
#include <QtWidgets>
#include <cmath>

class ViewerWidget : public QWidget
{
	Q_OBJECT

public:
	enum ObjectType
	{
		ObjectNone,
		ObjectLine,
		ObjectPolygon,
		ObjectTriangle
	};

	enum FillMode
	{
		FillNone,     
		FillSolidColor,
		FillNearestNeighbour,
		FillBarycentric
	};

private:
	ObjectType currentObjectType = ObjectNone;
	QVector<QPoint> objectPoints;

	bool drawPolygonActivated = false;
	bool dragging = false;
	bool clippingEnabled = true;
	QPoint lastMousePos = QPoint(0, 0);

	FillMode currentFillMode = FillNone;
	bool shapeFilled = false;
	QColor fillColor = Qt::blue;
	QColor triangleColor0 = Qt::red;
	QColor triangleColor1 = QColor(255, 170, 120);
	QColor triangleColor2 = Qt::blue;

	QImage* img = nullptr;
	uchar* data = nullptr;

	bool drawLineActivated = false;
	QPoint drawLineBegin = QPoint(0, 0);

	bool drawCircleActivated = false;
	QPoint drawCircleCenter = QPoint(0, 0);

public:
	ViewerWidget(QSize imgSize, QWidget* parent = Q_NULLPTR);
	~ViewerWidget();      
	void resizeWidget(QSize size);

	// Image functions
	bool setImage(const QImage& inputImg);
	QImage* getImage() { return img; }
	bool isEmpty();
	bool changeSize(int width, int height);

	void setPixel(int x, int y, int r, int g, int b, int a = 255);
	void setPixel(int x, int y, double valR, double valG, double valB, double valA = 1.);
	void setPixel(int x, int y, const QColor& color);
	bool isInside(int x, int y);

	// Cvik 2 drawing functions
	void drawLine(QPoint start, QPoint end, QColor color, int algType = 0);
	void drawCircle(QPoint center, int radius, QColor color);
	void drawCircleBresenham(QPoint center, int radius, QColor color);

	// Getters and setters
	void setDrawLineBegin(QPoint begin) { drawLineBegin = begin; }
	QPoint getDrawLineBegin() { return drawLineBegin; }
	void setDrawLineActivated(bool state) { drawLineActivated = state; }
	bool getDrawLineActivated() { return drawLineActivated; }

	void setDrawCircleCenter(QPoint center) { drawCircleCenter = center; }
	QPoint getDrawCircleCenter() { return drawCircleCenter; }
	void setDrawCircleActivated(bool state) { drawCircleActivated = state; }
	bool getDrawCircleActivated() { return drawCircleActivated; }

	ObjectType getCurrentObjectType() { return currentObjectType; }
	void setCurrentObjectType(ObjectType type) { currentObjectType = type; }

	QVector<QPoint> getObjectPoints() { return objectPoints; }
	void setObjectPoints(const QVector<QPoint>& pts) { objectPoints = pts; }

	void addObjectPoint(QPoint p) { objectPoints.push_back(p); }
	int getObjectPointCount() { return objectPoints.size(); }

	void setDrawPolygonActivated(bool state) { drawPolygonActivated = state; }
	bool getDrawPolygonActivated() { return drawPolygonActivated; }

	void setDragging(bool state) { dragging = state; }
	bool getDragging() { return dragging; }

	void setLastMousePos(QPoint p) { lastMousePos = p; }
	QPoint getLastMousePos() { return lastMousePos; }

	void setClippingEnabled(bool state) { clippingEnabled = state; }
	bool getClippingEnabled() { return clippingEnabled; }

	void setFillMode(FillMode mode) { currentFillMode = mode; }
	FillMode getFillMode() const { return currentFillMode; }

	void setShapeFilled(bool state) { shapeFilled = state; }
	bool getShapeFilled() const { return shapeFilled; }

	void setFillColor(const QColor& color) { fillColor = color; }
	QColor getFillColor() const { return fillColor; }

	void setTriangleColor0(const QColor& color) { triangleColor0 = color; }
	void setTriangleColor1(const QColor& color) { triangleColor1 = color; }
	void setTriangleColor2(const QColor& color) { triangleColor2 = color; }

	QColor getTriangleColor0() const { return triangleColor0; }
	QColor getTriangleColor1() const { return triangleColor1; }
	QColor getTriangleColor2() const { return triangleColor2; }

	bool isTriangle() const { return currentObjectType == ObjectTriangle; }
	bool isPolygon() const { return currentObjectType == ObjectPolygon; }

	// image access
	uchar* getData() { return data; }
	void setDataPtr() { data = img ? img->bits() : nullptr; }

	int getImgWidth() { return img ? img->width() : 0; }
	int getImgHeight() { return img ? img->height() : 0; }

	void clear();
	// Cvik 3 algorithms and functions
	void drawLineDDA(QPoint start, QPoint end, QColor color);
	void drawLineBresenham(QPoint start, QPoint end, QColor color);

	void clearAll();
	void redrawScene(QColor color, int algType);
	void drawCurrentObject(QColor color, int algType);
	void drawPolygon(const QVector<QPoint>& pts, QColor color, int algType);

	void translateObject(int dx, int dy);
	void rotateObject(double angleDeg);
	void scaleObject(double kx, double ky);
	void shearX(double shx);
	void symmetry();

	QPoint getReferencePoint() const;
	bool hasObject() const;
	bool isPointNearObject(QPoint p) const;
	
	//Clipping algorithms
	bool clipLineCyrusBeck(QPoint p1, QPoint p2, QPoint& out1, QPoint& out2) const;
	QVector<QPoint> clipPolygonSutherlandHodgman(const QVector<QPoint>& polygon) const;

	// Cvik 4 algorithms and functions
	void fillCurrentShape();
	void fillPolygonScanLine(const QVector<QPoint>& pts, const QColor& color);

	void fillTriangleSolid(const QVector<QPoint>& pts, const QColor& color);
	void fillTriangleInterpolated(const QVector<QPoint>& pts, FillMode mode);

	QColor getNearestNeighbourColor(const QPoint& p,
		const QVector<QPoint>& pts,
		const QColor& c0,
		const QColor& c1,
		const QColor& c2) const;

	QColor getBarycentricColor(const QPoint& p,
		const QVector<QPoint>& pts,
		const QColor& c0,
		const QColor& c1,
		const QColor& c2) const;
private:
	double distancePointToSegment(QPoint p, QPoint a, QPoint b) const;
	bool isInsideClipEdge(const QPoint& p, int edge, int boundary) const;
	QPoint intersectWithClipEdge(const QPoint& s, const QPoint& e, int edge, int boundary) const;

	struct ScanLineEdge
	{
		int yMax;
		double x;
		double invM;
	};
	QVector<QVector<ScanLineEdge>> createEdgeTable(const QVector<QPoint>& pts, int& yMin, int& yMax) const;

	void sortTriangleVertices(QVector<QPoint>& pts) const;

	void fillTopFlatTriangle(const QPoint& v0, const QPoint& v1, const QPoint& v2);
	void fillBottomFlatTriangle(const QPoint& v0, const QPoint& v1, const QPoint& v2);

	void drawTriangleScanLine(int y, double x1, double x2,
		const QVector<QPoint>& pts,
		FillMode mode);

	double triangleArea2(const QPoint& a, const QPoint& b, const QPoint& c) const;
public slots:
	void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
};