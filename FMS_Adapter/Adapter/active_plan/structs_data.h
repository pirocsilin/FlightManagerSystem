#ifndef STRUCT_DATA_H
#define STRUCT_DATA_H
#include <stdint.h>
#include <QVector>
#include <QQueue>
#include <map>
#include <unordered_map>
#include <functional>

//==============================================================================
enum MessageBitsDiffSrc: uint16_t
{
    BINS_DATA_DIFFERENCE    = 0x0001,   //!< Расхождение данных левого и правого БИНС
    IVSP_DATA_DIFFERENCE    = 0x0002,   //!< Расхождение данных левого и правого ИВСП
    ISRP_DATA_DIFFERENCE    = 0x0004,   //!< Расхождение данных 1го и 2го ИСРП
};

//==============================================================================
enum SourceData: uint32_t
{
    AUTO_SELECT_ENABLED = 0x00000001,

    BINSL_ENABLED  = 0x00000002,
    BINSL_NAV_SRC  = 0x00000004,
    BINSL_PIL_SRC  = 0x00000008,

    BINSR_ENABLED  = 0x00000010,
    BINSR_NAV_SRC  = 0x00000020,
    BINSR_PIL_SRC  = 0x00000040,

    DKV21L_ENABLED = 0x00000080,
    DKV21L_PIL_SRC = 0x00000100,

    DKV21R_ENABLED = 0x00000200,
    DKV21R_PIL_SRC = 0x00000400,

    IVSP1L_ENABLED = 0x00000800,
    IVSP1L_AIR_SRC = 0x00001000,

    IVSP1R_ENABLED = 0x00002000,
    IVSP1R_AIR_SRC = 0x00004000,

#if 0
    BINS1_ENABLED = 0x00000001,
    BINS2_ENABLED = 0x00000002,
    BINS1_NAV_SRC = 0x00000004,
    BINS2_NAV_SRC = 0x00000008,

    IVSP1_ENABLED = 0x00000010,
    IVSP2_ENABLED = 0x00000020,
    IVSP1_AIR_SRC = 0x00000040,
    IVSP2_AIR_SRC = 0x00000080,

    ISRP1_ENABLED = 0x00000100,
    ISRP2_ENABLED = 0x00000200,
    ISRP1_NAV_SRC = 0x00000400,
    ISRP1_AIR_SRC = 0x00000800,
    ISRP2_NAV_SRC = 0x00001000,
    ISRP2_AIR_SRC = 0x00002000,

    AUTO_SELECT_SELECTED = 0x00004000,
#endif
};

#if 0
//==============================================================================
/**
 * @brief Структура содержащая сведения для управления БНП.
 */
struct BnpQuery
{
    enum : int32_t {WRONG_FREQ=0};

    BnpQuery():
         freqVOR(WRONG_FREQ), freqILS(WRONG_FREQ), freqARF(WRONG_FREQ)
    {}

    //! Задает частоту VOR в соответствии с № канала
    inline bool setChannelVOR(int ch);
    //! Задает частоту ILS в соответствии с № канала
    inline bool setChannelILS(int ch);

    //! Убирает управление параметрами VOR, ILS, ARF
    inline void deleteAll() {	freqVOR = freqILS = freqARF = WRONG_FREQ;	}
    //! Убирает управление параметрами VOR
    inline void deleteVOR() {	freqVOR = WRONG_FREQ;	}
    //! Убирает управление параметрами ILS
    inline void deleteILS() {	freqILS = WRONG_FREQ;	}
    //! Убирает управление параметрами ARF
    inline void deleteARF() {	freqARF = WRONG_FREQ;	}

    //! Проверяет необходимость управления параметрами VOR
    inline bool isVOR() const {	return freqVOR != WRONG_FREQ;	}
    //! Проверяет необходимость управления параметрами ILS
    inline bool isILS() const {	return freqILS != WRONG_FREQ;	}
    //! Проверяет необходимость управления параметрами ARF
    inline bool isARF() const {	return freqARF != WRONG_FREQ;	}

    int32_t freqVOR;	//!< Частота VOR, Гц
    int32_t freqILS;	//!< Частота ILS, Гц
    int32_t freqARF;	//!< Частота ARF, Гц
}__attribute__((packed));

//------------------------------------------------------------------------------
/**
 * @param ch - номер канала от 1 до 160
 * @retval true - если частота задана
 */
inline bool BnpQuery::setChannelVOR(int ch)
{
    freqVOR = (ch >= 1 && ch < bnpFreqArrays::freqVor.size()) ? bnpFreqArrays::freqVor[ch] : WRONG_FREQ;
    return freqVOR != WRONG_FREQ;
}

//------------------------------------------------------------------------------
/**
 * @param ch - номер канала от 1 до 40
 * @retval true - если частота задана
 */
inline bool BnpQuery::setChannelILS(int ch)
{
    freqILS = (ch >= 1 && ch < bnpFreqArrays::freqIlsCourse.size()) ? bnpFreqArrays::freqIlsCourse[ch] : WRONG_FREQ;
    return freqILS != WRONG_FREQ;
}

#endif
//==============================================================================
/**
 * @brief Структура для управления ИВСП
 */
struct IvspQuery
{
    enum: quint32 {WRONG_CORRECTION = 0xFFFFFFFF};
    IvspQuery():
        baroCorrection1(WRONG_CORRECTION), baroCorrection2(WRONG_CORRECTION)
    {}

    //! Проверяет заданна ли барокоррекция1
    inline bool isCorrection1() const;
    //! Проверяет заданна ли барокоррекция2
    inline bool isCorrection2() const;

    quint32 baroCorrection1; //!< Барокоррекция 1, Па
    quint32 baroCorrection2; //!< Барокоррекция 2, Па
}__attribute__((packed));

//------------------------------------------------------------------------------
inline bool IvspQuery::isCorrection1() const
{
    return baroCorrection1 != WRONG_CORRECTION &&
           74500 <= baroCorrection1 && baroCorrection1 <= 105000;
}

//------------------------------------------------------------------------------
inline bool IvspQuery::isCorrection2() const
{
    return baroCorrection2 != WRONG_CORRECTION &&
           74500 <= baroCorrection2 && baroCorrection2 <= 105000;
}

//==============================================================================
/**
 * @brief Структура для ретрансляции данных от пультов
 */
struct ControlPadData
{
    enum TypeDataControlPad : uint8_t
    {
        CP_DATA_STATE  = 0,
        CP_DATA_ACTION,
    };

    struct ControlPadState
    {
        bool isMfcp;                //!< true - МФПУ, false - ПУИ
        bool isLeftSwitchPosition;  //!< true - левое положение, false - правое
        uint8_t brightnessTitle;    //!< яркость надписей (0 - 255)
        uint8_t brightnessKeys;     //!< яркость кнопок (0 - 255)
        bool isSetBacklightFsv;     //!< состояние лампочки ФСВ (true - горит)
        bool isSetBacklightRad;     //!< состояние лампочки РАД (true - горит)
        bool isSetBacklightNav;     //!< состояние лампочки НАВ (true - горит)
        bool isSetBacklightAnsw;    //!< состояние лампочки ОТВ (true - горит)
        float voltage;              //!< напряжение на пульте
    };

    struct ControlPadAction
    {
        bool isMfcp;            //!< true - МФПУ, false - ПУИ
        uint8_t idKey;          //!< id кнопки
        int8_t countRotation;   //!< количество вращений энкодера (отрицательное - против часовой)
        bool isPressed;         //!< true - кнопка нажата

    };

    union Payload
    {
        ControlPadState state;
        ControlPadAction action;
    };

    TypeDataControlPad type;
    Payload payload;
}__attribute__((packed));

struct CommandBrightnessControlPad
{
    char command[22];   //! максимальное сообщение в протоколе равно 21 байт для МФПУ
}__attribute__((packed));

namespace constantsTrueAirSpeed {
const std::map<int, double> tableAltitudeTemp{   {-1000, 294.7},
                                                 {-500, 291.4},
                                                 {0,    288.2},
                                                 {250,  286.55},
                                                 {500,  284.15},
                                                 {750,  283.25},
                                                 {1000, 281.65},
                                                 {1250, 280.05},
                                                 {1500, 278.40},
                                                 {1750, 276.75},
                                                 {2000, 275.15},
                                                 {2500, 271.95},
                                                 {3000, 268.65},
                                                 {3500, 265.45},
                                                 {4000, 262.15},
                                                 {4500, 258.95},
                                                 {5000, 255.65},
                                                 {5500, 252.45},
                                                 {6000, 249.25},
                                                 {6500, 245.95},
                                                 {7000, 242.70},
                                             };



constexpr double groundTemp{288.16};             //! градусы Кельвина
constexpr double altTempGradient{0.0065};   //! температурный градиент высоты, град/м
constexpr double airGasConstant{28.27};                 //! газовая постоянная воздуха, м/град
constexpr double groundPressure{760.0};                 //! давление у поверхности земли? мм рт. ст
}

//------------------------------------------------------------------------------
struct ChangeState
{
    enum StatusBits
    {
        ST_CHANGE_CUR_SCREEN = 0x00000001,  //!< признак изменения текущего кадра
        ST_CHANGE_STATE_PIL  = 0x00000002,  //!< признак изменения текущего состояния ПИЛ
        ST_CHANGE_STATE_TECH = 0x00000004,  //!< признак изменения текущего состояния ТЕХ
    };

    uint8_t curScreen;
    uint8_t curPilState;
    uint8_t curTechState;

    uint32_t statusBits{};     //!< достоверность полученных параметров
}__attribute__((__packed__));

//------------------------------------------------------------------------------

enum StatusBitsKeyIndex
{
    KEY_0  = 0x00000001,  //!< кнопка 0
    KEY_1  = 0x00000002,  //!< кнопка 1
    KEY_2  = 0x00000004,  //!< кнопка 2
    KEY_3  = 0x00000008,  //!< кнопка 3
    KEY_4  = 0x00000010,  //!< кнопка 4
    KEY_5  = 0x00000020,  //!< кнопка 5
    KEY_6  = 0x00000040,  //!< кнопка 6
    KEY_7  = 0x00000080,  //!< кнопка 7
    KEY_8  = 0x00000100,  //!< кнопка 8
    KEY_9  = 0x00000200,  //!< кнопка 9
    KEY_10 = 0x00000400,  //!< кнопка 10
    KEY_11 = 0x00000800,  //!< кнопка 11
};

struct ChangeStateButtons
{
    enum StatusBits
    {
        KEY_INDEX_PRESSED  = 0x00000001,  //!< индекс нажатой кнопки
        KEY_INDEX_RELEASED = 0x00000002,  //!< индекс отжатой кнопки
    };


    uint32_t indexPressedButton{};    //! индексы нажатых кнопок
    uint32_t indexReleasedButton{};   //! индексы отжатых кнопок
    uint8_t textKey[12]{};            //! массив с индексами надписей для кнопок

    uint32_t statusBits{};     //!< достоверность полученных параметров
}__attribute__((__packed__));

// ==========================================================================================

struct NavDataFp
{
    float longitude  {std::numeric_limits<float>::quiet_NaN()}; //!< долгота, радианы
    float latitude   {std::numeric_limits<float>::quiet_NaN()}; //!< широта, радианы
    float trackSpeed {std::numeric_limits<float>::quiet_NaN()}; //!< путевая скорость, м/с
};

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
        float azimuthToNextPPM        = 0.0f;
        float distanceToNextPPM       = 0.0f;
        float distanceToNextNextPPM   = 0.0f;
        float remainTimeToCurPoint    = 0.0f;
        float remainTimeToNextPoint   = 0.0f;
        float trackDeviationZ         = 0.0f;
        //
        float curSpeed                = 0.0f;
        float meanSpeed               = 0.0f;
    }

    void reset()
    {
        indexCurrentPoint   = 0;
        Vbufsize            = 100;      //!< Размер буфера для осреднения скорости для оценки времени прибытия
        switchPointDistance = 100;      //!< дальность для переключения на след ПМ, м
        Vnom                = 300/3.6;  //!< Номинальная скорость для расчета времени прибытия. 100 km/h пока так, потом решить
        Vbuf.clear();
        name.clear();
        points.clear();
    }

    void selectNextPoint(bool next)
    {
        if(next)
        {
            if(indexCurrentPoint < points.size()-1)
                ++indexCurrentPoint;
        }else
        {
            if(indexCurrentPoint != 0)
                --indexCurrentPoint;
        }
    }
};

#if 0
struct ActivePlanGuide
{
    struct PointParams
    {
        QString name;         //!< Название точки
        float   latitude;     //!< Широта точки  в WGS84, рад
        float   longitude;    //!< Долгота точки в WGS84, рад
        float   altitude;     //!< Высота точки  в WGS84, м
        quint32 LUR;          //!< Способ пролета точки, 0 - через точку, не 0 - с упреждением разворота
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

struct ActivePlanRouteGuide
{
    uint32_t  indexCurrentPoint;            //!< индекс текущей точки в маршруте, с 0
    float     trackDeviationZ;              //!< боковое уклонение ВС от линии заданного пути, м            **
    float     distanceTrackLineToCurPoint;  //!< расстояние вдоль линии пути до текущей точки, м
    float     azimuthToCurPoint;            //!< азимут от ВС на текущую точку, радианы                     **
    float     desiredTrackAngleToCurPoint;  //!< заданный путевой угол от ВС на текущую точку, радианы
    float     desiredAltitudeToCurPoint;    //!< заданная высота полета на текущую точку, м
    float     distanceToCurPoint;           //!< дальность до следующей точки, м                            **
    uint32_t  remainTimeToCurPoint;         //!< оставшееся время до прибытия в текущую точку, сек          **
    uint32_t  remainTimeToNextPoint;        //!< оставшееся время до прибытия в следующую точку, сек        **
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

    ActivePlanRouteGuide()
    {
        reset();
    }

    void reset()
    {
        indexCurrentPoint = 0;

        trackDeviationZ             = 0;
        distanceTrackLineToCurPoint = 0;
        azimuthToCurPoint           = 0;
        desiredTrackAngleToCurPoint = 0;
        desiredAltitudeToCurPoint   = 0;
        distanceToCurPoint          = 0;
        remainTimeToCurPoint        = 0;
        coursePlank     = 0;
        glissadePlank   = 0;
        isNav           = 0;
        isDirectTo      = 0;

        Kpsi = 0.3f;        //!< коэффициент для горизонтального индекса (было 0.3 для ограничения до -54..+54 градуса)
        Kz   = -0.2f;       //!< коэффициент для горизонтального индекса.

        Kh_nav = 0.1f;      //!< коэффициент вертикального индекса в режиме наведения
        Kh_stb = 0.1f;      //!< коэффициент вертикального индекса в режиме стабилизации
        Kv     = -8e-4f;    //!< коэффициент для вертикального индекса

        Vbufsize = 100;     //!< Размер буфера для осреднения скорости для оценки времени прибытия
        Vbuf.clear();

        switchPointDistance = 100;      //!< дальность для переключения на след ПМ, м
        Vnom                = 300/3.6;  //!< Номинальная скорость для расчета времени прибытия. 100 km/h пока так, потом решить
    }

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
#endif

// ==========================================================================================

enum class ModeMfiScreen
{
    SCREEN_NONE = 0,
    SCREEN_PIL,
    SCREEN_NAV,
    SCREEN_TECH,
};

enum class TypeStatePil : uint8_t
{
    PIL_STATE_NONE = 0,
    PIL_STATE_MAIN,
    PIL_STATE_SET_COURSE,
    PIL_STATE_SET_TRACK_ANGLE,

};

#endif // STRUCT_DATA_H
