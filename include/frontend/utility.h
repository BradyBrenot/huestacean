#pragma once

//Qt helpers and misc. things for frontend


//////////////////////////////////////////////////////////
// The _easiest_, but not typesafe, way of getting custom
// C++ types in and out of QML seems to be as QVariant, 
// which is fine as long as they're known to Qt's type
// system.
//
// You can do QList<QVariant> (QVariantList) too, these
// are treated as JS arrays of QVariant inside QML.
//

template<typename T>
QVariantList makeVariantList(const QList<T>& list)
{
	auto vl = QVariantList();
	for (const auto& item : list)
	{
		vl.push_back(QVariant::fromValue(item));
	}
	return vl;
}

template<typename T>
QList<T> fromVariantList(const QVariantList& in)
{
	auto out = QList<T>{};
	for (const auto& item : in)
	{
		out.push_back(item.value<T>());
	}
	return out;
}
//////////////////////////////////////////////////////////