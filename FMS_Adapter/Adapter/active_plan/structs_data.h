#ifndef STRUCT_DATA_H
#define STRUCT_DATA_H

#include <QVector>
#include <QQueue>

struct ActivePlanGuide
{
    struct PointParams
    {
        QString name;            //!< Название точки
        float   latitude;        //!< Широта точки  в WGS84, рад
        float   longitude;       //!< Долгота точки в WGS84, рад
        float   altitude;        //!< Высота точки  в WGS84, м
    };

    QString  name;                //!< Название маршрута
    uint32_t indexCurrentPoint;   //!< индекс текущей точки в маршруте, с 0
    float    switchPointDistance; //!< Дальность для переключения на след. ППМ, метры
    float    Vnom;                //!< Номинальная скорость для расчета времени прибытия (если тек. скорость меньше этой, берем эту) m/s
    quint32  Vbufsize;            //!< Размер буфера для осредения скорости для оценки времени прибытия
    QVector<PointParams> points;  //!< Точки маршрута
    QQueue<float>        Vbuf;    //!< Буфер для осреднения скорости

    float azimuthToNextPPM        {0.0f};     //!< азимут от ВС на след ППМ, рад
    float distanceToNextPPM       {0.0f};     //!< дальность от ВС на след ППМ, м
    float distanceToNextNextPPM   {0.0f};     //!< дальность от ВС на след после активной ППМ, м
    float remainTimeToCurPoint    {0.0f};     //!< оставшееся время до прибытия в текущую точку, сек
    float remainTimeToNextPoint   {0.0f};     //!< оставшееся время до прибытия в следующую точку, сек
    float trackDeviationZ         {0.0f};     //!< боковое отклонение от линии заданного пути, м
    //
    float curSpeed                {0.0f};     //!< текущая скорость ВС, м/с
    float meanSpeed               {0.0f};     //!< средняя скорость ВС, м/с

    ActivePlanGuide() { reset(); }

    void clearData()
    {
        name.clear();
        points.clear();
        indexCurrentPoint       = 0;
        //
        azimuthToNextPPM        = 0.0f;
        distanceToNextPPM       = 0.0f;
        distanceToNextNextPPM   = 0.0f;
        remainTimeToCurPoint    = 0.0f;
        remainTimeToNextPoint   = 0.0f;
        trackDeviationZ         = 0.0f;
        //
        curSpeed                = 0.0f;
        meanSpeed               = 0.0f;
    }

    void reset()
    {
        indexCurrentPoint   = 0;
        Vbufsize            = 100;      //!< Размер буфера для осреднения скорости для оценки времени прибытия
        switchPointDistance = 100;      //!< дальность для переключения на след ПМ, м
        Vnom                = 100/3.6;  //!< Номинальная скорость для расчета времени прибытия. 100 km/h пока так, потом решить
        Vbuf.clear();
        name.clear();
        points.clear();
    }

    bool selectNextPoint(bool next)
    {
        if(next)
        {
            if(indexCurrentPoint < points.size()-1)
            {
                ++indexCurrentPoint;
                return true;
            }
            return false;
        }else
        {
            if(indexCurrentPoint != 0)
            {
                --indexCurrentPoint;
                return true;
            }
            return false;
        }
    }
};

#endif // STRUCT_DATA_H
