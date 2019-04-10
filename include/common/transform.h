#pragma once

#include <QVector3D>
#include <QQuaternion>

class Transform
{
	QVector3D location;
	QVector3D scale;
	QQuaternion quaternion;
};

class Box
{
	QVector3D center;
	QVector3D halfSize;
};