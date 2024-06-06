#ifndef PLAYER_H
#define PLAYER_H

#include <QMap>
#include <QObject>
#include <QSet>

#include "unit.h"


/**
 * @brief The Player class
 * Each player has a list of APP6 for every single unit in the game
 * When a player view is changed or when a unit is changed, update this
 * This class will emit a playerDataChanged signal and the view will
 * update accordingly (to be specific, the QGraphicsScene)
 */
class Player : public QObject
{
    Q_OBJECT
    enum PLAYER_RELATIONSHIP{
        SELF,
        ALLY,
        NEUTRAL,
        ENEMY
    };

    ///////////////////////////////////////////////////////////////////////
    /// CONSTRUCTOR, DESTRUCTOR, INIT...                                ///
    ///////////////////////////////////////////////////////////////////////
public:
    Player(const QString & name, QObject * parent = nullptr);
    void clearVisibility();


    ///////////////////////////////////////////////////////////////////////
    /// GETTERS, SETTERS                                                ///
    ///////////////////////////////////////////////////////////////////////
public:
    QString name() const;
    void setName(const QString &name);


//    QMap<QString, PLAYER_RELATIONSHIP> relationships() const;
//    void setRelationships(const QMap<QString, PLAYER_RELATIONSHIP> &relationships);
//    QStringList allies() const;
//    void addAlly(const QString & allyName);

    QMap<int, QStringList> visibleUnitInfo() const;
    void setVisibleUnitInfo(const QMap<int, QStringList> &visibleUnitInfo);

    QSet<int> ownedUnits() const;
    void setOwnedUnits(const QSet<int> &ownedUnits);

    Unit::AFFILIATION affiliation() const;
    QString affiliationString() const;
    void setAffiliation(const QString & letter);

    ///////////////////////////////////////////////////////////////////////
    /// BLA                                                             ///
    ///////////////////////////////////////////////////////////////////////
public:
    void setVisible(int uniqueId, QStringList info);
    void setInvisible(int uniqueId);

public:
    void debug();

    ///////////////////////////////////////////////////////////////////////
    /// ATTRIBUTES                                                      ///
    ///////////////////////////////////////////////////////////////////////


private:
    QString mName;
    //    QMap<QString, PLAYER_RELATIONSHIP> mRelationships;

    QMap<int, QStringList> mVisibleUnitInfo;    // What the player sees
    // Only includes stuff that is seen by the player
    // Pair of strings : APP6 and displayed text

    QSet<int> mOwnedUnits;
    QMap<int, int> mDelayedIntelligence; // Go find in Data where they were X turns ago
    QMap<int, QStringList> mDetections;

    Unit::AFFILIATION mAffiliation;
};

#endif // PLAYER_H
