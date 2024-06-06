#include "player.h"

#include <QDebug>

Player::Player(const QString & name, QObject * parent):
    QObject(parent),
    mName(name)
{

}

void Player::clearVisibility()
{
    mVisibleUnitInfo.clear();
}

QString Player::name() const
{
    return mName;
}

void Player::setName(const QString &name)
{
    mName = name;
}

//QMap<QString, Player::PLAYER_RELATIONSHIP> Player::relationships() const
//{
//    return mRelationships;
//}

//void Player::setRelationships(const QMap<QString, PLAYER_RELATIONSHIP> &relationships)
//{
//    mRelationships = relationships;
//}

QMap<int, QStringList> Player::visibleUnitInfo() const
{
    return mVisibleUnitInfo;
}

void Player::setVisibleUnitInfo(const QMap<int, QStringList> &visibleUnitApp6)
{
    mVisibleUnitInfo = visibleUnitApp6;
}

Unit::AFFILIATION Player::affiliation() const
{
    return mAffiliation;
}

QString Player::affiliationString() const
{
    switch (mAffiliation) {
    case Unit::FRIENDLY:
        return QString("F");
    case Unit::HOSTILE:
        return QString("H");
    case Unit::NEUTRAL:
        return QString("N");
    default:
        return QString("U");
    }
}

void Player::setAffiliation(const QString &letter)
{
    if(letter == QString("F")){
        mAffiliation = Unit::FRIENDLY;
        return;
    }
    if(letter == QString("H")){
        mAffiliation = Unit::HOSTILE;
        return;
    }
    if(letter == QString("N")){
        mAffiliation = Unit::NEUTRAL;
        return;
    }
    mAffiliation = Unit::UNKNOWN_AFFILIATION;

}

//QStringList Player::allies() const
//{
//    QStringList answer;

//    for(auto it = mRelationships.constBegin(); it != mRelationships.constEnd(); ++it){
//        if(it.value() == Player::ALLY){
//            answer.push_back(it.key());
//        }
//    }

//    return answer;
//}

//void Player::addAlly(const QString &allyName)
//{
//    mRelationships
//}

void Player::setVisible(int uniqueId, QStringList info)
{
    mVisibleUnitInfo[uniqueId] = info;
}

void Player::setInvisible(int uniqueId)
{
    mVisibleUnitInfo.remove(uniqueId);
}

void Player::debug()
{
    for(auto it = mVisibleUnitInfo.cbegin(); it != mVisibleUnitInfo.cend(); ++it){
//        qDebug() << mName << " sees " << it.key() << " with APP6 " << it.value();
    }
}

QSet<int> Player::ownedUnits() const
{
    return mOwnedUnits;
}

void Player::setOwnedUnits(const QSet<int> &ownedUnits)
{
    mOwnedUnits = ownedUnits;
}
