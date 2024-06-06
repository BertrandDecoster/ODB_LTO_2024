#ifndef unit_H
#define unit_H

#include <QString>
#include <QMetaType>
#include <QVector>
#include <QIcon>

QT_BEGIN_NAMESPACE
class QJsonObject;
class QVariant;
QT_END_NAMESPACE

class Unit
{
    ///////////////////////////////////////////////////////////////////////
    /// ENUMS                                                           ///
    ///////////////////////////////////////////////////////////////////////
public:
    enum AFFILIATION{
        UNKNOWN_AFFILIATION,        //-
        FRIENDLY,                   //F
        HOSTILE,                    //H
        NEUTRAL                     //N
    };

    enum FUNCTION{
        UNKNOWN_FUNCTION,           //----
        ARMOR,                      //CA--
        ANTIARMOR,                  //CAA-
        ANTIARMOR_SPECIAL,             //CAB-
        ARMOR_WHEELED,              //CAW-
        AIR_DEFENSE,                //CD--
        AIR_DEFENSE_MISSILE,        //CDM-
        ENGINEER,                   //CE--
        ENGINEER_TRACKED,           //CECT
        ARTILLERY,                  //CF--
        ARTILLERY_SELF_PROPELLED,   //CFHE
        ARTILLERY_TARGET_DISMOUNTED,//CFTCD
        ARTILLERY_TARGET_MOUNTED,   //CFTCM
        MORTAR,                     //CFM
        INFANTRY,                   //CI--
        INFANTRY_FIGHTING_VEHICLE,  //CI--
        INFANTRY_MOTORIZED,         //CIM-
        INFANTRY_MECHANIZED,        //CIZ-
        RECONNAISSANCE,             //CR--
        RECONNAISSANCE_ARMORED,     //CRVA
        RECONNAISSANCE_MOTORIZED,   //CRVM
        FLYING,                     //CV--
        FLYING_UAV,                 //CVU-
        SUPPLY,                     //SS--
        TRANSPORTATION,             //ST--
        MAINTENANCE,                //SX--
        SIGNALS,                    //US--
    };

    enum SIZE{
        UNKNOWN_SIZE,               //-
        TEAM,                       //A
        SQUAD,                      //B
        SECTION,                    //C
        PLATOON,                    //D
        COMPANY,                    //E
        BATTALION,                  //F
        REGIMENT,                   //G
        BRIGADE,                    //H
        DIVISION,                   //I
        CORPS,                      //J
        ARMY,                       //K
    };

    ///////////////////////////////////////////////////////////////////////
    /// CONSTRUCTORS AND DESTRUCTOR                                     ///
    ///////////////////////////////////////////////////////////////////////
public:
    Unit();
    Unit(int uniqueId);
    Unit(const Unit& unit);     // Be extremely careful, because the uniqueId is copied
    ~Unit();

//    Unit& operator=(const Unit& unit);              // Copy
    Unit& operator=(Unit other);                    // Copy
    bool operator==(const Unit& unit) const;        // Copy
    bool operator!=(const Unit& unit) const;        // Copy
    Unit createChild(int childId) const;            // Child



    ///////////////////////////////////////////////////////////////////////
    /// GETTERS AND SETTERS                                             ///
    ///////////////////////////////////////////////////////////////////////
public:
    int uniqueId() const;
    void setUniqueId(int uniqueId);

    QString displayedText() const;
    void setDisplayedText(const QString &value);

    QString app6() const;
    void setApp6(const QString &app6);

    Unit::AFFILIATION affiliation() const;
    void setAffiliation(Unit::AFFILIATION affiliation);

    Unit::FUNCTION function() const;
    void setFunction(Unit::FUNCTION function);

    Unit::SIZE size() const;
    void setSize(Unit::SIZE size);

    int parentId() const;
    void setParent(int parentId);

    QVector<int> children() const;
    void setChildren(const QVector<int> & children);
    void clearChildren();
    void addChild(int childId);
    void insertChild(int childId, int position = -1);
    int removeChild(int childId);
    bool hasChild(int childId);
    bool hasChildren();

    double x() const;
    void setX(double x);

    double y() const;
    void setY(double y);

    QPointF pos() const;
    void setPos(QPointF pos);
    bool isPosLegal() const;

    float getLife() const;
    void setLife(float life);

    QString owner() const;
    void setOwner(const QString &value);

    ///////////////////////////////////////////////////////////////////////
    /// SERIALIZATION                                                   ///
    ///////////////////////////////////////////////////////////////////////
public:
    void readJson(const QJsonObject &json);
    void writeJson(QJsonObject & json) const;

//    QList<QString> contentHeaders() const;
//    QVariant buildQVariant(bool excludeHierarchy = true) const;
//    QVector<QVariant> content() const;
//    int contentSize() const;
//    void setupContent(const QVariant &content);


    ///////////////////////////////////////////////////////////////////////
    /// MISC                                                            ///
    ///////////////////////////////////////////////////////////////////////
public:

    QIcon getIcon();
    QIcon getIconWithLife();
    operator QString() const;

    QString app6ToNoAffiliation() const;
    QString app6ToNoSize() const;
    QString app6ToNoAffiliationNoFunction() const;
    QString app6ToNoAffiliationNoSize() const;
    QString app6ToUnknown() const;



    ///////////////////////////////////////////////////////////////////////
    /// ATTRIBUTES                                                      ///
    ///////////////////////////////////////////////////////////////////////
private:
    int mUniqueId;
    QString mDisplayedText; // The unit name, displayed on the bottom right
    QString mOptionalText;
    QString mApp6;


    int mParent;            // uniqueId
    QVector<int> mChildren; // uniqueId

    double mX;              // Latitude
    double mY;              // Longitude
    float mLife;

    QString mOwner;


    QIcon mIcon;
    bool mIsIconValid;

    QIcon mIconWithLife;
    bool mIsIconWithLifeValid;

    ///////////////////////////////////////////////////////////////////////
    /// STATIC                                                          ///
    ///////////////////////////////////////////////////////////////////////
public:
    static Unit::AFFILIATION getAffiliation(QChar affiliationApp6Char);
    static Unit::AFFILIATION getAffiliation(const QString & affiliationName);
    static QChar getAffiliationApp6(Unit::AFFILIATION affiliation);
    static QString getAffiliationName(Unit::AFFILIATION affiliation);
    static QString setAffiliation(const QString & app6, Unit::AFFILIATION affiliation);

    static Unit::FUNCTION getFunction(const QString & function);
    static QString getFunctionApp6(Unit::FUNCTION function);
    static QString getFunctionName(Unit::FUNCTION function);
    static QString setFunction(const QString & app6, Unit::FUNCTION function);


    static Unit::SIZE getSize(QChar sizeApp6Char);
    static Unit::SIZE getSize(const QString & sizeName);
    static QChar getSizeApp6(Unit::SIZE size);
    static QString getSizeName(Unit::SIZE size);
    static QString setSize(const QString & app6, Unit::SIZE size);

    static const QVector<Unit::AFFILIATION> affiliations;
    static const QVector<Unit::FUNCTION> functions;
    static const QVector<Unit::SIZE> sizes;

    static const QMap<QString, Unit::FUNCTION> functionMapNameToEnum;
    static const QMap<QString, Unit::FUNCTION> functionMapApp6ToEnum;
    static const QMap<Unit::FUNCTION, QString> functionMapEnumToApp6;

    static bool isApp6Legal(const QString & app6);
    static bool isApp6Present(const QString & app6);

    static int iconWidth;
    static int iconHeight;
    static int iconTextWidth;
    static int iconTextHeight;
    static int iconTotalWidth;
    static int iconTotalHeight;

    static int ILLEGAL_POSITION;
    static bool isPosLegal(QPointF pos);
    static QPointF illegalPosition();

    static int UNIQUE_ID_GENERATOR;


public:
    // For more explanation, refer to :
    // https://stackoverflow.com/questions/3279543/what-is-the-copy-and-swap-idiom#3279550
    friend void swap(Unit & first, Unit & second)
    {
        using std::swap;
        swap(first.mUniqueId, second.mUniqueId);
        swap(first.mDisplayedText, second.mDisplayedText);
        swap(first.mOptionalText, second.mOptionalText);
        swap(first.mApp6, second.mApp6);
        swap(first.mParent, second.mParent);
        swap(first.mChildren, second.mChildren);
        swap(first.mX, second.mX);
        swap(first.mY, second.mY);
        swap(first.mLife, second.mLife);
        swap(first.mOwner, second.mOwner);
        swap(first.mIcon, second.mIcon);
        swap(first.mIsIconValid, second.mIsIconValid);
    }

};

Q_DECLARE_METATYPE(Unit);

#endif // unit_H
