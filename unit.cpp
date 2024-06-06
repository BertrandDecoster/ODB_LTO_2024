#include "unit.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QVariant>
#include <QDebug>
#include <QFile>
#include <QIcon>
#include <QPainter>
#include <QObject>

#include <cmath>


///////////////////////////////////////////////////////////////////////
/// STATIC                                                          ///
///////////////////////////////////////////////////////////////////////
const QVector<Unit::AFFILIATION> Unit::affiliations (
{
            UNKNOWN_AFFILIATION,        //-
            FRIENDLY,                   //F
            HOSTILE,                    //H
            NEUTRAL                     //N
        });

const QVector<Unit::FUNCTION> Unit::functions (
{
            UNKNOWN_FUNCTION,           //----
            ARMOR,                      //CA--
            ARMOR_WHEELED,              //CAW-
            INFANTRY,                   //CI--
            INFANTRY_FIGHTING_VEHICLE,  //CI--
            INFANTRY_MOTORIZED,         //CIM-
            INFANTRY_MECHANIZED,        //CIZ-
            ARTILLERY,                  //CF--
            ARTILLERY_SELF_PROPELLED,   //CFHE
            ARTILLERY_TARGET_DISMOUNTED,//CFTCD
            ARTILLERY_TARGET_MOUNTED,   //CFTCM
            MORTAR,                     //CFM
            RECONNAISSANCE,             //CR--
            RECONNAISSANCE_ARMORED,     //CRVA
            RECONNAISSANCE_MOTORIZED,   //CRVM
            FLYING,                     //CV--
            FLYING_UAV,                 //CVU-
            ANTIARMOR,                  //CAA-
            ANTIARMOR_SPECIAL,          //CAB-
            AIR_DEFENSE,                //CD--
            AIR_DEFENSE_MISSILE,        //CDM-
            ENGINEER,                   //CE--
            ENGINEER_TRACKED,           //CECT
            SUPPLY,                     //SS--
            TRANSPORTATION,             //ST--
            MAINTENANCE,                //SX--
            SIGNALS,                    //US--
        });

const QMap<QString, Unit::FUNCTION> Unit::functionMapNameToEnum(
{
            {QStringLiteral("ANTI ARMOR"), Unit::ANTIARMOR},
            {QStringLiteral("ANTI ARMOR SPECIAL"), Unit::ANTIARMOR_SPECIAL},
            {QStringLiteral("ARMOR"), Unit::ARMOR},
            {QStringLiteral("ARMOR WHEELED"), Unit::ARMOR_WHEELED},
            {QStringLiteral("INFANTRY"), Unit::INFANTRY},
            {QStringLiteral("INFANTRY FIGHTING VEHICLE"), Unit::INFANTRY_FIGHTING_VEHICLE},
            {QStringLiteral("INFANTRY MOTORIZED"), Unit::INFANTRY_MOTORIZED},
            {QStringLiteral("INFANTRY MECHANIZED"), Unit::INFANTRY_MECHANIZED},
            {QStringLiteral("ARTILLERY"), Unit::ARTILLERY},
            {QStringLiteral("ARTILLERY SELF PROPELLED"), Unit::ARTILLERY_SELF_PROPELLED},
            {QStringLiteral("ARTILLERY TARGET DISMOUNTED"), Unit::ARTILLERY_TARGET_DISMOUNTED},
            {QStringLiteral("ARTILLERY TARGET MOUNTED"), Unit::ARTILLERY_TARGET_MOUNTED},
            {QStringLiteral("MORTAR"), Unit::MORTAR},
            {QStringLiteral("RECONNAISSANCE"), Unit::RECONNAISSANCE},
            {QStringLiteral("RECONNAISSANCE MOTORIZED"), Unit::RECONNAISSANCE_MOTORIZED},
            {QStringLiteral("RECONNAISSANCE ARMORED"), Unit::RECONNAISSANCE_ARMORED},
            {QStringLiteral("AIR DEFENSE"), Unit::AIR_DEFENSE},
            {QStringLiteral("AIR DEFENSE MISSILE"), Unit::AIR_DEFENSE_MISSILE},
            {QStringLiteral("FLYING"), Unit::FLYING},
            {QStringLiteral("UAV"), Unit::FLYING_UAV},
            {QStringLiteral("ENGINEER"), Unit::ENGINEER},
            {QStringLiteral("ENGINEER TRACKED"), Unit::ENGINEER_TRACKED},
            {QStringLiteral("SUPPLY"), Unit::SUPPLY},
            {QStringLiteral("TRANSPORTATION"), Unit::TRANSPORTATION},
            {QStringLiteral("MAINTENANCE"), Unit::MAINTENANCE},
            {QStringLiteral("SIGNALS"), Unit::SIGNALS}
        });

const QMap<QString, Unit::FUNCTION> Unit::functionMapApp6ToEnum(
{
            {QStringLiteral("CAA--"), Unit::ANTIARMOR},
            {QStringLiteral("CAB--"), Unit::ANTIARMOR_SPECIAL},
            {QStringLiteral("CA---"), Unit::ARMOR},
            {QStringLiteral("CAW--"), Unit::ARMOR_WHEELED},
            {QStringLiteral("CI---"), Unit::INFANTRY},
            {QStringLiteral("CII--"), Unit::INFANTRY_FIGHTING_VEHICLE},
            {QStringLiteral("CIM--"), Unit::INFANTRY_MOTORIZED},
            {QStringLiteral("CIZ--"), Unit::INFANTRY_MECHANIZED},
            {QStringLiteral("CF---"), Unit::ARTILLERY},
            {QStringLiteral("CFTCD"), Unit::ARTILLERY_TARGET_DISMOUNTED},
            {QStringLiteral("CFTCM"), Unit::ARTILLERY_TARGET_MOUNTED},
            {QStringLiteral("CFHE-"), Unit::ARTILLERY_SELF_PROPELLED},
            {QStringLiteral("CFM--"), Unit::MORTAR},
            {QStringLiteral("CR---"), Unit::RECONNAISSANCE},
            {QStringLiteral("CRVM-"), Unit::RECONNAISSANCE_MOTORIZED},
            {QStringLiteral("CRVA-"), Unit::RECONNAISSANCE_ARMORED},
            {QStringLiteral("CD---"), Unit::AIR_DEFENSE},
            {QStringLiteral("CDM--"), Unit::AIR_DEFENSE_MISSILE},
            {QStringLiteral("CV---"), Unit::FLYING},
            {QStringLiteral("CVU--"), Unit::FLYING_UAV},
            {QStringLiteral("CE---"), Unit::ENGINEER},
            {QStringLiteral("CECT-"), Unit::ENGINEER_TRACKED},
            {QStringLiteral("SS---"), Unit::SUPPLY},
            {QStringLiteral("ST---"), Unit::TRANSPORTATION},
            {QStringLiteral("SX---"), Unit::MAINTENANCE},
            {QStringLiteral("US---"), Unit::SIGNALS},
        });

const QMap<Unit::FUNCTION, QString> Unit::functionMapEnumToApp6(
{
            {Unit::ANTIARMOR,                  QStringLiteral("CAA--")},
            {Unit::ANTIARMOR_SPECIAL,          QStringLiteral("CAB--")},
            {Unit::ARMOR,                      QStringLiteral("CA---")},
            {Unit::ARMOR_WHEELED,              QStringLiteral("CAW--")},
            {Unit::INFANTRY,                   QStringLiteral("CI---")},
            {Unit::INFANTRY_FIGHTING_VEHICLE,  QStringLiteral("CII--")},
            {Unit::INFANTRY_MOTORIZED,         QStringLiteral("CIM--")},
            {Unit::INFANTRY_MECHANIZED,        QStringLiteral("CIZ--")},
            {Unit::ARTILLERY,                  QStringLiteral("CF---")},
            {Unit::ARTILLERY_TARGET_DISMOUNTED,QStringLiteral("CFTCD")},
            {Unit::ARTILLERY_TARGET_MOUNTED,   QStringLiteral("CFTCM")},
            {Unit::ARTILLERY_SELF_PROPELLED,   QStringLiteral("CFHE-")},
            {Unit::MORTAR,                     QStringLiteral("CFM--")},
            {Unit::RECONNAISSANCE,             QStringLiteral("CR---")},
            {Unit::RECONNAISSANCE_MOTORIZED,   QStringLiteral("CRVM-")},
            {Unit::RECONNAISSANCE_ARMORED,     QStringLiteral("CRVA-")},
            {Unit::AIR_DEFENSE,                QStringLiteral("CD---")},
            {Unit::AIR_DEFENSE_MISSILE,        QStringLiteral("CDM--")},
            {Unit::FLYING,                     QStringLiteral("CV---")},
            {Unit::FLYING_UAV,                 QStringLiteral("CVU--")},
            {Unit::ENGINEER,                   QStringLiteral("CE---")},
            {Unit::ENGINEER_TRACKED,           QStringLiteral("CECT-")},
            {Unit::SUPPLY,                     QStringLiteral("SS---")},
            {Unit::TRANSPORTATION,             QStringLiteral("ST---")},
            {Unit::MAINTENANCE,                QStringLiteral("SX---")},
            {Unit::SIGNALS,                    QStringLiteral("US---")}
        });

const QVector<Unit::SIZE> Unit::sizes ({UNKNOWN_SIZE,               //-
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
                                       });
//int Unit::unitCount = 0;
int Unit::iconWidth = 200;
int Unit::iconHeight = 200;
int Unit::iconTextWidth = 175;
int Unit::iconTextHeight = 24;
int Unit::iconTotalWidth = Unit::iconWidth + Unit::iconTextWidth;
int Unit::iconTotalHeight = Unit::iconHeight;

int Unit::ILLEGAL_POSITION = -424242;
int Unit::UNIQUE_ID_GENERATOR = 1000;

///////////////////////////////////////////////////////////////////////
/// CONSTRUCTORS AND DESTRUCTOR                                     ///
///////////////////////////////////////////////////////////////////////
Unit::Unit():
    mUniqueId(-1),
    mApp6(QStringLiteral("SFGPU-------")),
    mParent(-1),
    mX(0),
    mY(0),
    mLife(100),
    mOwner(QStringLiteral("")),
    mIcon(QIcon()),
    mIsIconValid(false),
    mIconWithLife(QIcon()),
    mIsIconWithLifeValid(false)
{
    //    mParent = QStringLiteral("NOPARENT");
    //    mUniqueId = QStringLiteral("Unit #") + QString::number(Unit::unitCount);
    //    unitCount++;
}

Unit::Unit(int uniqueId): Unit()
{
    mUniqueId = uniqueId;
}

// Dangerous because the uniqueId is now duplicated
Unit::Unit(const Unit &unit):
    mUniqueId(unit.mUniqueId),
    mDisplayedText(unit.mDisplayedText),
    mOptionalText(unit.mOptionalText),
    mApp6(unit.mApp6),
    mParent(unit.mParent),
    mChildren(unit.mChildren),
    mX(unit.mX),
    mY(unit.mY),
    mLife(unit.mLife),
    mOwner(unit.mOwner),
    mIcon(QIcon()),
    mIsIconValid(false),
    mIconWithLife(QIcon()),
    mIsIconWithLifeValid(false)
{

}

Unit::~Unit(){

}

//Unit &Unit::operator=(const Unit &unit)
//{

//    mUniqueId = unit.mUniqueId;
//    mDisplayedText = unit.mDisplayedText;
//    mOptionalText = unit.mOptionalText;
//    mApp6 = unit.mApp6;
//    mParent = unit.mParent;
//    mChildren = unit.mChildren;
//    mX = unit.mX;
//    mY = unit.mY;
//    mOwner = unit.mOwner;
//    mIcon = QIcon();
//    mIsIconValid = false;

//    return *this;
//}

Unit &Unit::operator=(Unit other)
{
    swap(*this, other);
    return *this;
}

bool Unit::operator==(const Unit & unit) const
{
    if(mUniqueId != unit.mUniqueId){return false;}
    if(mDisplayedText != unit.mDisplayedText){return false;}
    if(mOptionalText != unit.mOptionalText){return false;}
    if(mApp6 != unit.mApp6){return false;}
    if(mParent != unit.mParent){return false;}
    if(mChildren != unit.mChildren){return false;}
    if(mX != unit.mX){return false;}
    if(mY != unit.mY){return false;}
    if(mLife != unit.mLife){return false;}
    if(mOwner != unit.mOwner){return false;}

    return true;
}

bool Unit::operator!=(const Unit &unit) const
{
    return !(*this == unit);
}



//Unit::Unit(const QVariant &content)
//{
//    setupContent(content);
//}

Unit Unit::createChild(int childId) const
{
    //    mChildren.append(childId);

    Unit answer;
    answer.setUniqueId(childId);
    answer.setDisplayedText(QString::number(childId));
    answer.setParent(mUniqueId);
    //    answer.setParent(-1);

    double theta = mChildren.size()*3.14159265*2.0*2.0/11.1;
    double distance = 20;
    answer.setX(mX+distance*cos(theta));
    answer.setY(mY+distance*sin(theta));
    answer.setLife(mLife);

    // App6
    QChar parentDimension = mApp6.back();
    QChar childDimension = QChar(parentDimension.unicode() - 1);
    if(parentDimension == 'H'){     // By default, brigades' children are battalions
        childDimension = 'F';
    }
    if(parentDimension == 'A'){
        childDimension = 'A';
    }
    if(parentDimension == '-'){
        childDimension = '-';
    }
    QString childApp6 = mApp6.mid(0, 11) + childDimension;
    answer.setApp6(childApp6);

    answer.setOwner(mOwner);

    return answer;
}




///////////////////////////////////////////////////////////////////////
/// GETTERS AND SETTERS                                             ///
///////////////////////////////////////////////////////////////////////
int Unit::uniqueId() const
{
    return mUniqueId;
}

void Unit::setUniqueId(int uniqueId)
{
    mUniqueId = uniqueId;
}


QString Unit::displayedText() const
{
    return mDisplayedText;
}

void Unit::setDisplayedText(const QString &value)
{
    mDisplayedText = value;
    mIsIconValid = false;
}

QString Unit::app6() const
{
    return mApp6;
}

void Unit::setApp6(const QString &app6)
{
    mApp6 = app6;
    mIsIconValid = false;
}

Unit::AFFILIATION Unit::affiliation() const
{
    if(mApp6.size() < 3){
        return AFFILIATION::UNKNOWN_AFFILIATION;
    }
    QChar c = mApp6[1];
    return getAffiliation(c);
}

void Unit::setAffiliation(Unit::AFFILIATION affiliation)
{
    mApp6 = setAffiliation(mApp6, affiliation);
    mIsIconValid = false;
}

Unit::FUNCTION Unit::function() const
{
    if(mApp6.size() < 10){
        return FUNCTION::UNKNOWN_FUNCTION;
    }
    QString function = mApp6.mid(5,5);
    return getFunction(function);
}

void Unit::setFunction(Unit::FUNCTION function)
{
    mApp6 = setFunction(mApp6, function);
    mIsIconValid = false;
}

Unit::SIZE Unit::size() const
{
    if(mApp6.size() < 12){
        return SIZE::UNKNOWN_SIZE;
    }
    QChar c = mApp6[11];
    return getSize(c);
}

void Unit::setSize(Unit::SIZE size)
{
    mApp6 = setSize(mApp6, size);
    mIsIconValid = false;
}

int Unit::parentId() const
{
    return mParent;
}

void Unit::setParent(int parentId)
{
    mParent = parentId;
}

QVector<int> Unit::children() const
{
    return mChildren;
}

void Unit::setChildren(const QVector<int> & children)
{
    mChildren = children;
}

void Unit::clearChildren()
{
    mChildren.clear();
}

void Unit::addChild(int childId)
{
    mChildren.push_back(childId);
}

void Unit::insertChild(int childId, int position)
{
    int truePosition = position;
    if(position<0 || position > mChildren.size()){
        truePosition = mChildren.size();
    }
    mChildren.insert(truePosition, childId);
}

int Unit::removeChild(int childId)
{
    return mChildren.removeAll(childId);
}

bool Unit::hasChild(int childId)
{
    return mChildren.contains(childId);
}

bool Unit::hasChildren()
{
    return !(mChildren.empty());
}

double Unit::x() const
{
    return mX;
}

void Unit::setX(double x)
{
    mX = x;
}

double Unit::y() const
{
    return mY;
}

void Unit::setY(double y)
{
    mY = y;
}

QPointF Unit::pos() const
{
    return QPointF(mX, mY);
}

void Unit::setPos(QPointF pos)
{
    mX = pos.x();
    mY = pos.y();
}

bool Unit::isPosLegal() const
{
    return Unit::isPosLegal(QPointF(mX, mY));
}

QString Unit::owner() const
{
    return mOwner;
}

void Unit::setOwner(const QString &value)
{
    mOwner = value;
}

///////////////////////////////////////////////////////////////////////
/// SERIALIZATION                                                   ///
///////////////////////////////////////////////////////////////////////
void Unit::readJson(const QJsonObject &json)
{
    if (json.contains("uniqueId") && json["uniqueId"].isDouble()){

        mUniqueId = json["uniqueId"].toDouble();
    }
        else{
        mUniqueId = Unit::UNIQUE_ID_GENERATOR;
        Unit::UNIQUE_ID_GENERATOR++;
    }
    if (json.contains("name") && json["name"].isString()){
        mDisplayedText = json["name"].toString();
    }
    else if (json.contains("displayedText") && json["displayedText"].isString()){
        mDisplayedText = json["displayedText"].toString();
    }
    else if (json.contains("uniqueId") && json["uniqueId"].isString()){
        mDisplayedText = QString::number(json["uniqueId"].toInt());
    }



    if (json.contains("optionalText") && json["optionalText"].isString())
        mOptionalText = json["optionalText"].toString();

    if (json.contains("app6") && json["app6"].isString())
        mApp6 = json["app6"].toString();

    if (json.contains("parent") && json["parent"].isDouble()){
        mParent = static_cast<int>(json["parent"].toDouble());
    }
    else{
        mParent = -1;
    }

    if (json.contains("children") && json["children"].isArray()){
        mChildren.clear();
        QJsonArray childrenArray = json["children"].toArray();
        mChildren.reserve(childrenArray.size());
        for(int i=0; i<childrenArray.size(); ++i){
            mChildren.append(childrenArray[i].toInt());
        }
    }

    if (json.contains("pos") && json["pos"].isArray()){
        QJsonArray posArray = json["pos"].toArray();
        mX = posArray[0].toDouble();
        mY = posArray[1].toDouble();
    }
    else{
        mX = Unit::ILLEGAL_POSITION;
        mY = Unit::ILLEGAL_POSITION;
    }

    if (json.contains("life") && json["life"].isDouble()){
        mLife = json["life"].toDouble();
    }
    else{
        mLife = 100;
    }

    if (json.contains("owner") && json["owner"].isString())
        mOwner = json["owner"].toString();
}

void Unit::writeJson(QJsonObject &json) const
{
    json["uniqueId"] = mUniqueId;

    if(mDisplayedText != QString::number(mUniqueId)){
        json["displayedText"] = mDisplayedText;
    }

    if(!mOptionalText.isEmpty())
        json["optionalText"] = mOptionalText;

    if(!mApp6.isEmpty())
        json["app6"] = mApp6;

    if(mParent>=0)
        json["parent"] = mParent;

    if(!mChildren.empty()){
        QJsonArray childrenArray;
        for(int childId : mChildren){
            childrenArray.append(QJsonValue{childId});
        }
        json["children"] = childrenArray;
    }

    if(isPosLegal()){
        QJsonArray posArray;
        posArray.append(QJsonValue{mX});
        posArray.append(QJsonValue{mY});
        json["pos"] = posArray;
    }

    if(mLife != 100.0f){
        json["life"] = static_cast<double>(mLife);
    }

    if(!mOwner.isEmpty()){
        json["owner"] = mOwner;
    }
}



///////////////////////////////////////////////////////////////////////
/// MISC                                                            ///
///////////////////////////////////////////////////////////////////////
// TODO : Utiliser QFontMetrics
QIcon Unit::getIcon()
{
    if(!mIsIconValid){
        QString actualApp6 = mApp6;
        QString filepath = ":/files/app6/";
        filepath += mApp6;
        filepath += ".svg";
        // Change filepath if it fails
        bool isFunctionPresent = Unit::isApp6Present(mApp6);
        if(!isFunctionPresent){
            // Build valid app6
            QString tempApp6 = QStringLiteral("S-GPU-------");
            tempApp6 = setAffiliation(tempApp6, affiliation());
            tempApp6 = setSize(tempApp6, size());
            //            qDebug() << "Temp app6 " << tempApp6;
            actualApp6 = tempApp6;
            filepath = ":/files/app6/";
            filepath += tempApp6;
            filepath += ".svg";
        }
        mIcon = QIcon(filepath);
        QPixmap svgPixmap = mIcon.pixmap(QSize(iconWidth, iconHeight)); // The .svg file, i.e the real
        //        qDebug() << "Icon " << mUniqueId << " " << pixmap.size(); // NO COMMENT THIS // LOL I COMMENTED IT

        QPixmap fullPixmap = QPixmap(iconTotalWidth, iconTotalHeight); // Contains the extra text, lifebar, etc...
        fullPixmap.fill(Qt::transparent);
        QPainter painter(&fullPixmap);
        painter.drawPixmap(svgPixmap.rect(), svgPixmap, svgPixmap.rect());
        painter.setPen(Qt::black);

        if(!isFunctionPresent){
            QString centerText = mApp6.mid(5, 5);
            centerText.remove(QChar('-'));
            //            qDebug() << "Draw text : " << centerText;
            painter.setFont(QFont("Arial", iconHeight/8, QFont::Bold));
            int marginX = iconWidth/10;
            painter.drawText(QRectF(marginX, (iconHeight*7)/16,
                                    iconWidth-2*marginX, iconHeight/4),
                             Qt::AlignCenter, centerText);
        }

        QString displayText = mDisplayedText;
        if(mDisplayedText.size()>8){
            displayText = QString("..") + mDisplayedText.right(6);
        }
        painter.setFont(QFont("Arial", iconTextHeight));
        QFont font = painter.font();
        font.setWeight(QFont::Bold);
        painter.setFont(font);
        painter.drawText(QRectF(iconWidth, svgPixmap.height()-2*iconTextHeight,
                                iconTextWidth, iconTextHeight*1.5),
                         Qt::AlignLeft, displayText);
        mIcon = QIcon(fullPixmap);
        mIsIconValid = true;
    }


    return mIcon;
}

QIcon Unit::getIconWithLife()
{
    if(!mIsIconWithLifeValid){
        QIcon icon = getIcon();
        QPixmap pixmap = icon.pixmap(QSize(iconTotalWidth, iconTotalHeight));

        QPainter painter(&pixmap);
        {
            QPen pen;
            pen.setStyle(Qt::NoPen);
            painter.setPen(pen);
            QBrush brush;
            brush.setColor(Qt::black);
            brush.setStyle(Qt::SolidPattern);
            painter.setBrush(brush);
        }
        float marginX = iconWidth/30.0;
        float marginY = iconHeight/4.0;
        painter.drawRect(QRectF(iconWidth + marginX, marginY,
                                2.5*marginX, iconHeight/3.0));

        {
            QBrush brush;
//            brush.setColor(Qt::green);
            double factor = 1.0-(mLife/100.0)*(mLife/100.0);
            brush.setColor(QColor(255*(factor), 255*(1.0-factor), 0.));
            brush.setStyle(Qt::SolidPattern);
            painter.setBrush(brush);
        }
        float borderSize = 3.0f;
        float bottomLifeBar = marginY + iconHeight/3.0 - borderSize;
        float topLifeBarMax = marginY+borderSize;
        float heightLifeBar = (bottomLifeBar-topLifeBarMax)*mLife/100.0;
        painter.drawRect(QRectF(iconWidth + marginX+borderSize, bottomLifeBar - heightLifeBar ,
                                2.5*marginX-2*borderSize, heightLifeBar));

        mIconWithLife = QIcon(pixmap);
        mIsIconWithLifeValid = true;
    }

    return mIconWithLife;
}

QString Unit::app6ToNoAffiliation() const
{
    QString answer = mApp6;
    answer = setAffiliation(answer, Unit::UNKNOWN_AFFILIATION);
    return answer;
}

QString Unit::app6ToNoSize() const
{
    QString answer = mApp6;
    answer = setSize(answer, Unit::UNKNOWN_SIZE);
    return answer;
}

QString Unit::app6ToNoAffiliationNoFunction() const
{
    QString answer = app6ToNoAffiliation();
    answer = setFunction(answer, Unit::UNKNOWN_FUNCTION);
    return answer;
}

QString Unit::app6ToNoAffiliationNoSize() const
{
    QString answer = app6ToNoAffiliation();
    answer = setSize(answer, Unit::UNKNOWN_SIZE);
    return answer;
}

QString Unit::app6ToUnknown() const
{
    QString answer = app6ToNoAffiliationNoFunction();
    answer = setSize(answer, Unit::UNKNOWN_SIZE);
    return answer;
}

float Unit::getLife() const
{
    return mLife;
}

void Unit::setLife(float life)
{
    float correctedLife = life;
    if(life<0.0f){
        correctedLife = 0.0f;
    }
    if(life > 100.0f){
        correctedLife = 100.0f;
    }
    mLife = correctedLife;
}



Unit::operator QString() const
{
    return QString("unit : \n\tUnique Id: ") + QString::number(mUniqueId) +
            "\n\tDisplay: " + mDisplayedText +
            "\n\tApp6: " + mApp6 +
            "\n\tPos: (" + QString::number(mX, 'g') + ",\t" + QString::number(mY, 'g') + ")" +
            "\n\tLife: " + QString::number(mLife) +
            "\n\tParent: " + QString::number(mParent) +
            "\n\tNb of Children: " + QString::number(mChildren.size()) +
            "\n\tOwner: " + mOwner +
            "\n";
}

///////////////////////////////////////////////////////////////////////
/// STATIC                                                          ///
///////////////////////////////////////////////////////////////////////
Unit::AFFILIATION Unit::getAffiliation(QChar affiliationApp6Char)
{
    switch(affiliationApp6Char.unicode()){
    case 'F':
        return AFFILIATION::FRIENDLY;
    case 'H':
        return AFFILIATION::HOSTILE;
    case 'N':
        return AFFILIATION::NEUTRAL;
    default:
        return AFFILIATION::UNKNOWN_AFFILIATION;
    }
}

Unit::AFFILIATION Unit::getAffiliation(const QString & affiliationName)
{
    if(affiliationName == QString("Friend"))
        return AFFILIATION::FRIENDLY;

    if(affiliationName == QString("Hostile"))
        return AFFILIATION::HOSTILE;

    if (affiliationName == QString("Neutral"))
        return AFFILIATION::NEUTRAL;

    return AFFILIATION::UNKNOWN_AFFILIATION;

}

QChar Unit::getAffiliationApp6(Unit::AFFILIATION affiliation)
{
    switch(affiliation){
    case AFFILIATION::FRIENDLY:
        return 'F';
    case AFFILIATION::HOSTILE:
        return 'H';
    case AFFILIATION::NEUTRAL:
        return 'N';
    case AFFILIATION::UNKNOWN_AFFILIATION:
        return 'U';
    }
    // To remove the compiler warning
    return 'U';
}

QString Unit::getAffiliationName(Unit::AFFILIATION affiliation)
{
    switch(affiliation){
    case AFFILIATION::FRIENDLY:
        return QString("Friend");
    case AFFILIATION::HOSTILE:
        return QString("Hostile");
    case AFFILIATION::NEUTRAL:
        return QString("Neutral");
    case AFFILIATION::UNKNOWN_AFFILIATION:
        return QString("Unknown");
    }
    // To remove the compiler warning
    return "Unknown";
}

QString Unit::setAffiliation(const QString &app6, Unit::AFFILIATION affiliation)
{
    QString answer;
    answer.fill('-', 12);
    answer.replace(0, app6.size(), app6);
    answer = answer.left(12);
    answer[1] = getAffiliationApp6(affiliation);
    return answer;
}

Unit::FUNCTION Unit::getFunction(const QString &function)
{
    if(functionMapApp6ToEnum.contains(function)){
        return functionMapApp6ToEnum.value(function);
    }
    if(functionMapNameToEnum.contains(function.toUpper())){
        return functionMapNameToEnum.value(function.toUpper());
    }
    return Unit::UNKNOWN_FUNCTION;
}

QString Unit::getFunctionApp6(Unit::FUNCTION function)
{
    if(functionMapEnumToApp6.contains(function)){
        return functionMapEnumToApp6.value(function);
    }
    return QStringLiteral("-----");
}

QString Unit::getFunctionName(Unit::FUNCTION function)
{
    switch(function){
    case ANTIARMOR:
        return QObject::tr("Anti Armor");
    case ANTIARMOR_SPECIAL:
        return QObject::tr("Anti Armor Special");
    case ARMOR:
        return QObject::tr("Armor");
    case ARMOR_WHEELED:
        return QObject::tr("Armor Wheeled");
    case INFANTRY:
        return QObject::tr("Infantry");
    case INFANTRY_FIGHTING_VEHICLE:
        return QObject::tr("Infantry Fighting Vehicle");
    case INFANTRY_MOTORIZED:
        return QObject::tr("Infantry Motorized");
    case INFANTRY_MECHANIZED:
        return QObject::tr("Infantry Mechanized");
    case ARTILLERY:
        return QObject::tr("Artillery");
    case ARTILLERY_TARGET_DISMOUNTED:
        return QObject::tr("Artillery Target Dismounter");
    case ARTILLERY_TARGET_MOUNTED:
        return QObject::tr("Artillery Target Mounted");
    case ARTILLERY_SELF_PROPELLED:
        return QObject::tr("Artillery Self Propelled");
    case MORTAR:
        return QObject::tr("Mortar");
    case RECONNAISSANCE:
        return QObject::tr("Reconnaissance");
    case RECONNAISSANCE_MOTORIZED:
        return QObject::tr("Reconnaissance Motorized");
    case RECONNAISSANCE_ARMORED:
        return QObject::tr("Reconnaissance Armored");
    case AIR_DEFENSE:
        return QObject::tr("Air Defense");
    case AIR_DEFENSE_MISSILE:
        return QObject::tr("Air Defense Missile");
    case FLYING:
        return QObject::tr("Flying");
    case FLYING_UAV:
        return QObject::tr("Flying UAV");
    case ENGINEER:
        return QObject::tr("Engineer");
    case ENGINEER_TRACKED:
        return QObject::tr("Engineer Tracked");
    case SUPPLY:
        return QObject::tr("Supply");
    case TRANSPORTATION:
        return QObject::tr("Transportation");
    case MAINTENANCE:
        return QObject::tr("Maintenance");
    case SIGNALS:
        return QObject::tr("Signals");
    default:
        return QObject::tr("Unknown Function");
    }
}

QString Unit::setFunction(const QString &app6, Unit::FUNCTION function)
{
    QString answer;
    answer.fill('-', 12);
    answer.replace(0, app6.size(), app6);
    answer = answer.left(12);
    answer.replace(5, 5, getFunctionApp6(function));
    return answer;
}

Unit::SIZE Unit::getSize(QChar sizeApp6Char)
{
    switch(sizeApp6Char.unicode()){
    case 'A':
        return SIZE::TEAM;
    case 'B':
        return SIZE::SQUAD;
    case 'C':
        return SIZE::SECTION;
    case 'D':
        return SIZE::PLATOON;
    case 'E':
        return SIZE::COMPANY;
    case 'F':
        return SIZE::BATTALION;
    case 'G':
        return SIZE::REGIMENT;
    case 'H':
        return SIZE::BRIGADE;
    case 'I':
        return SIZE::DIVISION;
    default:
        return SIZE::UNKNOWN_SIZE;
    }
}

Unit::SIZE Unit::getSize(const QString &sizeName)
{

    if (sizeName == QString("Team"))
        return SIZE::TEAM;
    if (sizeName == QString("Squad"))
        return SIZE::SQUAD;
    if (sizeName == QString("Section"))
        return  SIZE::SECTION;
    if (sizeName == QString("Platoon"))
        return  SIZE::PLATOON;
    if (sizeName == QString("Company"))
        return  SIZE::COMPANY;
    if (sizeName == QString("Battalion"))
        return  SIZE::BATTALION;
    if (sizeName == QString("Regiment"))
        return  SIZE::REGIMENT;
    if (sizeName == QString("Brigade"))
        return  SIZE::BRIGADE;
    if (sizeName == QString("Division"))
        return  SIZE::DIVISION;
    if (sizeName == QString("Corps"))
        return  SIZE::CORPS;
    if (sizeName == QString("Army"))
        return  SIZE::ARMY;

    return SIZE::UNKNOWN_SIZE;

}

QChar Unit::getSizeApp6(Unit::SIZE size)
{
    switch(size){
    case SIZE::UNKNOWN_SIZE:
        return QChar('-');
    case SIZE::TEAM:
        return QChar('A');
    case SIZE::SQUAD:
        return QChar('B');
    case SIZE::SECTION:
        return QChar('C');
    case SIZE::PLATOON:
        return QChar('D');
    case SIZE::COMPANY:
        return QChar('E');
    case SIZE::BATTALION:
        return QChar('F');
    case SIZE::REGIMENT:
        return QChar('G');
    case SIZE::BRIGADE:
        return QChar('H');
    case SIZE::DIVISION:
        return QChar('I');
    case SIZE::CORPS:
        return QChar('J');
    case SIZE::ARMY:
        return QChar('K');
    }
    // To remove the compiler warning
    return QChar('-');
}

QString Unit::getSizeName(Unit::SIZE size)
{
    switch(size){
    case SIZE::UNKNOWN_SIZE:
        return QObject::tr("-");
    case SIZE::TEAM:
        return QObject::tr("Team");
    case SIZE::SQUAD:
        return QObject::tr("Squad");
    case SIZE::SECTION:
        return QObject::tr("Section");
    case SIZE::PLATOON:
        return QObject::tr("Platoon");
    case SIZE::COMPANY:
        return QObject::tr("Company");
    case SIZE::BATTALION:
        return QObject::tr("Battalion");
    case SIZE::REGIMENT:
        return QObject::tr("Regiment");
    case SIZE::BRIGADE:
        return QObject::tr("Brigade");
    case SIZE::DIVISION:
        return QObject::tr("Division");
    case SIZE::CORPS:
        return QObject::tr("Corps");
    case SIZE::ARMY:
        return QObject::tr("Army");
    }
    // To remove the compiler warning
    return QObject::tr("-");
}

QString Unit::setSize(const QString &app6, Unit::SIZE size)
{
    QString answer;
    answer.fill('-', 12);
    answer.replace(0, app6.size(), app6);
    answer = answer.left(12);
    answer[11] = getSizeApp6(size);
    return answer;
}

bool Unit::isApp6Legal(const QString &app6)
{
    if(app6.size() != 12)
        return false;

    if(app6[0] != 'S')
        return false;

    if(app6[2] != 'G')
        return false;

    if(app6[3] != 'P')
        return false;

    if(app6[4] != 'U')
        return false;

    return true;
}

bool Unit::isApp6Present(const QString &app6)
{
    QString filepath = ":/files/app6/";
    filepath += app6;
    filepath += ".svg";
    QFile file{filepath};
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    return true;
}

bool Unit::isPosLegal(QPointF pos)
{
    return (pos.x() != Unit::ILLEGAL_POSITION && pos.y() != Unit::ILLEGAL_POSITION);

}

QPointF Unit::illegalPosition()
{
    return QPointF(Unit::ILLEGAL_POSITION, Unit::ILLEGAL_POSITION);
}




