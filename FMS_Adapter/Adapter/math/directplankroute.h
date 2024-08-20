#ifndef DIRECTPLANKROUTE_H
#define DIRECTPLANKROUTE_H

#include <QVector>
#include <QQueue>

struct DirectPlankRoute
{
    struct PointParams
    {
        QString name;         //!< Название точки
        float   latitude;     //!< Широта точки  в WGS84, рад
        float   longitude;    //!< Долгота точки в WGS84, рад
        float   altitude;     //!< Высота точки  в WGS84, м
        quint32 LUR;          //!< Способ пролета точки, 0 - через точку, не 0 - с упреждением разворота
//        float   slide;        //!< Требуемое значение угла скольжения, радианы
//        float   pitch;        //!< Требуемое значение тангажа, радианы
//        float   roll;         //!< Требуемое значение крена, радианы
        quint32 pointType;    //!< Тип точки (разворота, разгона, контроля, входа в зону, выхода из зоны)

        //! расчетные данные
        float   distanceFromPrevPoint;     //!< Расстояние до этой точки от предыдущей, м
        float   distanceFromCurPosition;   //!< Расстояние до этой точки от текущего положения, м
        quint32 timeSecFromPrevPoint;      //!< Время прибытия до точки от предыдущей, с
        quint32 timeSecFromCurPosition;    //!< Время прибытия до точки от текущего положения, с
    };

    QString name;                   //!< Название маршрута
    QVector<PointParams> points;    //!< Точки маршрута

    void clear()
    {
        name.clear();
        points.clear();
    }
};

struct DirectPlankRouteGuide
{
    uint32_t  indexCurrentPoint;            //!< индекс текущей точки в маршруте, с 0
    float     trackDeviationZ;              //!< боковое уклонение ВС от линии заданного пути, м
    float     distanceTrackLineToCurPoint;  //!< расстояние вдоль линии пути до текущей точки, м
    float     azimuthToCurPoint;            //!< азимут от ВС на текущую точку, радианы
    float     desiredTrackAngleToCurPoint;  //!< заданный путевой угол от ВС на текущую точку, радианы
    float     desiredAltitudeToCurPoint;    //!< заданная высота полета на текущую точку, м
    float     distanceToCurPoint;           //!< дальность до следующей точки, м
    uint32_t  remainTimeToCurPoint;         //!< оставшееся время до прибытия в текущую точку, сек
    float     coursePlank;                  //!< горизонтальный директорный индекс, у.е.
    float     glissadePlank;                //!< вертикальный директорный индекс, градусы
    uint32_t  isNav;                        //!< 1 - в режиме наведение, 0 - стабилизация
    uint32_t  isDirectTo;                   //!< первую точку летим в режиме "Прямо На"

    // параметры управления
    float    Kpsi;       //!< Коэффициент для горизонтального индекса
    float    Kz;         //!< Коэффициент для горизонтального индекса
    float    Kh_nav;     //!< Коэффициент для вертикального индекса в режиме "Наведение"
    float    Kh_stb;     //!< Коэффициент для вертикального индекса в режиме "Стабилизация"
    float    Kv;         //!< Коэффициент для вертикального индекса
    float    switchPointDistance;       //!< Дальность для переключения на след. ППМ, метры
    float    Vnom;       //!< Номинальная скорость для расчета времени прибытия (если тек. скорость меньше этой, берем эту) m/s

    quint32 Vbufsize;    //!< Размер буфера для осредения скорости для оценки времени прибытия
    QQueue<float> Vbuf;  //!< Буфер для осреднения скорости

    DirectPlankRouteGuide();
    void reset();

    /** Переключение на след/пред точку маршрута
     *
     * При достижении первой точки дальше не переключает.
     * Проверка верхней границы точек выполняется в routeGuide, так как там есть доступ к полному маршруту.
     * \param [in] next - если true - то переключаем на след точку, false - на предыдущую
     */
    void selectNextPoint(bool next)
    {
        if(next)
        {
            ++indexCurrentPoint;
        }else
        {
            if(indexCurrentPoint != 0)
                --indexCurrentPoint;
        }
    }
};

#endif // DIRECTPLANKROUTE_H
