
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
typedef qintptr nativeEvent_qintptr;
typedef float QColor_HsvF_type;

#define QLocale_territory territory
#define QVariant_typeId typeId
#define enterEvent_QEnterEvent QEnterEvent
#define QMouseEvent_globalPosition_toPoint globalPosition().toPoint

#define QStringListNE(x, y) (x!=y)
#else
typedef long nativeEvent_qintptr;
typedef qreal QColor_HsvF_type;

#define QLocale_territory country
#define QVariant_typeId type
#define enterEvent_QEnterEvent QEvent
#define QMouseEvent_globalPosition_toPoint globalPos
#define QStringListNE(x, y) [&x,&y]()\
{\
    if (x.size() != y.size()) {\
        return true;\
}\
else {\
    for (int i = 0; i < x.size(); ++i) {\
        if (x[i] != y[i]) {\
            return true;\
        }\
    }\
    return false;\
}\
}()
#endif
