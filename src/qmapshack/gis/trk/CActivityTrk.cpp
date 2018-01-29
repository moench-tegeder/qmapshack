/**********************************************************************************************
    Copyright (C) 2014-2015 Oliver Eichler oliver.eichler@gmx.de

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

**********************************************************************************************/

#include "gis/trk/CActivityTrk.h"
#include "gis/trk/CGisItemTrk.h"
#include "helpers/CSettings.h"
#include "units/IUnit.h"

QVector<CActivityTrk::desc_t> CActivityTrk::actDescriptor;


const CActivityTrk::desc_t CActivityTrk::dummyDesc =
{
    "-"
    , CTrackData::trkpt_t::eActNone
    , "-"
    , "://icons/48x48/ActNone.png"
    , "://icons/16x16/ActNone.png"
    , QColor()
};

CActivityTrk::CActivityTrk(CGisItemTrk * trk)
    : trk(trk)
    , allFlags(0)
{
}


void CActivityTrk::init()
{
    actDescriptor =
    {
        { // 0
            "Foot"
            , CTrackData::trkpt_t::eActFoot
            , tr("Foot")
            , "://icons/48x48/ActFoot.png"
            , "://icons/16x16/ActFoot.png"
            , IGisItem::colorMap[0].color
        },
        { // 1
            "Cycle"
            , CTrackData::trkpt_t::eActCycle
            , tr("Bicycle")
            , "://icons/48x48/ActCycle.png"
            , "://icons/16x16/ActCycle.png"
            , IGisItem::colorMap[1].color
        },
        { // 2
            "Bike"
            , CTrackData::trkpt_t::eActBike
            , tr("Motor Bike")
            , "://icons/48x48/ActBike.png"
            , "://icons/16x16/ActBike.png"
            , IGisItem::colorMap[2].color
        },
        { // 3
            "Car"
            , CTrackData::trkpt_t::eActCar
            , tr("Car")
            , "://icons/48x48/ActCar.png"
            , "://icons/16x16/ActCar.png"
            , IGisItem::colorMap[3].color
        },
        { // 4
            "Cable"
            , CTrackData::trkpt_t::eActCable
            , tr("Cable Car")
            , "://icons/48x48/ActCable.png"
            , "://icons/16x16/ActCable.png"
            , IGisItem::colorMap[4].color
        },
        { // 5
            "Swim"
            , CTrackData::trkpt_t::eActSwim
            , tr("Swim")
            , "://icons/48x48/ActSwim.png"
            , "://icons/16x16/ActSwim.png"
            , IGisItem::colorMap[5].color
        },
        { // 6
            "Ship"
            , CTrackData::trkpt_t::eActShip
            , tr("Ship")
            , "://icons/48x48/ActShip.png"
            , "://icons/16x16/ActShip.png"
            , IGisItem::colorMap[6].color
        },
        { // 7
            "Aeronautik"
            , CTrackData::trkpt_t::eActAero
            , tr("Aeronautics")
            , "://icons/48x48/ActAero.png"
            , "://icons/16x16/ActAero.png"
            , IGisItem::colorMap[7].color
        },
        { // 8
            "Ski/Winter"
            , CTrackData::trkpt_t::eActSki
            , tr("Ski/Winter")
            , "://icons/48x48/ActSki.png"
            , "://icons/16x16/ActSki.png"
            , IGisItem::colorMap[8].color
        },
        { // 9
            "Train"
            , CTrackData::trkpt_t::eActTrain
            , tr("Public Transport")
            , "://icons/48x48/ActTrain.png"
            , "://icons/16x16/ActTrain.png"
            , IGisItem::colorMap[9].color
        }
    };



    SETTINGS;
    cfg.beginGroup("Activities");
    int i = 0;
    for(desc_t &desc : actDescriptor)
    {
        desc.color = QColor(cfg.value(QString("color%1").arg(i), desc.color.name()).toString());
        ++i;
    }
    cfg.endGroup(); // Activities
}

void CActivityTrk::release()
{
    SETTINGS;
    cfg.beginGroup("Activities");
    int i = 0;
    for(desc_t &desc : actDescriptor)
    {
        cfg.setValue(QString("color%1").arg(i), desc.color.name());
        ++i;
    }
    cfg.endGroup(); // Activities
}

quint32 CActivityTrk::selectActivity(QWidget *parent)
{
    QMenu menu(parent);
    QAction * act;

    act = menu.addAction(QIcon("://icons/32x32/ActNone.png"), tr("No Activity"));
    act->setData(QVariant(CTrackData::trkpt_t::eActNone));

    for(const desc_t &desc : actDescriptor)
    {
        act = menu.addAction(QIcon(desc.iconLarge), desc.name);
        act->setData(QVariant(desc.flag));
    }

    act = menu.exec(QCursor::pos());
    if(nullptr != act)
    {
        return act->data().toUInt(nullptr);
    }

    return 0xFFFFFFFF;
}


void CActivityTrk::update()
{
    allFlags = 0;
    activityRanges.clear();
    activitySummary.clear();

    const CTrackData&   data       = trk->getTrackData();
    const CTrackData::trkpt_t *lastTrkpt  = nullptr;
    const CTrackData::trkpt_t *startTrkpt = nullptr;

    quint32 lastFlag = 0xFFFFFFFF;
    for(const CTrackData::trkpt_t &pt : data)
    {
        allFlags |= pt.flags;

        if(pt.isHidden())
        {
            continue;
        }
        lastTrkpt = &pt;
        if((pt.flags & CTrackData::trkpt_t::eActMask) != lastFlag)
        {
            if(startTrkpt != nullptr)
            {
                activity_summary_t& summary = activitySummary[lastFlag];
                summary.distance += pt.distance - startTrkpt->distance;
                summary.ascent   += pt.ascent   - startTrkpt->ascent;
                summary.descent  += pt.descent  - startTrkpt->descent;
                summary.ellapsedSeconds += pt.elapsedSeconds - startTrkpt->elapsedSeconds;
                summary.ellapsedSecondsMoving += pt.elapsedSecondsMoving - startTrkpt->elapsedSecondsMoving;

                activityRanges << activity_range_t();
                activity_range_t& activity = activityRanges.last();

                activity.d1 = startTrkpt->distance;
                activity.d2 = pt.distance;
                activity.t1 = startTrkpt->time.toTime_t();
                activity.t2 = pt.time.toTime_t();

                const desc_t& desc = getDescriptor(lastFlag);
                activity.name = desc.name;
                activity.icon = desc.iconSmall;
            }

            startTrkpt  = &pt;
            lastFlag    = pt.flags & CTrackData::trkpt_t::eActMask;
        }
    }

    if(lastTrkpt == nullptr)
    {
        return;
    }

    activity_summary_t& summary = activitySummary[lastFlag];
    summary.distance += lastTrkpt->distance - startTrkpt->distance;
    summary.ascent   += lastTrkpt->ascent   - startTrkpt->ascent;
    summary.descent  += lastTrkpt->descent  - startTrkpt->descent;
    summary.ellapsedSeconds += lastTrkpt->elapsedSeconds - startTrkpt->elapsedSeconds;
    summary.ellapsedSecondsMoving += lastTrkpt->elapsedSecondsMoving - startTrkpt->elapsedSecondsMoving;

    activityRanges << activity_range_t();
    activity_range_t& activity = activityRanges.last();

    activity.d1 = startTrkpt->distance;
    activity.d2 = lastTrkpt->distance;
    activity.t1 = startTrkpt->time.toTime_t();
    activity.t2 = lastTrkpt->time.toTime_t();

    const desc_t& desc = getDescriptor(lastFlag);
    activity.name = desc.name;
    activity.icon = desc.iconSmall;



    allFlags &= CTrackData::trkpt_t::eActMask;

//    for(int i = 0; i < 9; i++)
//    {
//        activity_summary_t& stat   = summaries[i];
//        qDebug() << "--------------" << i << "--------------";
//        qDebug() << "stat.distance" << stat.distance;
//        qDebug() << "stat.ascent" << stat.ascent;
//        qDebug() << "stat.descent" << stat.descent;
//        qDebug() << "stat.timeMoving" << stat.ellapsedSecondsMoving;
//        qDebug() << "stat.timeTotal" << stat.ellapsedSeconds;
//    }
}

void CActivityTrk::printSummary(QString& str) const
{
    printSummary(activitySummary, allFlags, str);
}

void CActivityTrk::printSummary(const QMap<quint32, activity_summary_t>& summary, quint32 flags, QString& str)
{
    QString val, unit;
    qreal total;
    qreal distance;
    bool printTotal = false;
    bool printNoAct = false;

    str += "<table>";

    // gather any used activities
    QVector<const desc_t*> descs;
    for(const desc_t &desc : actDescriptor)
    {
        if(flags & desc.flag)
        {
            descs << &desc;
        }
    }

    const activity_summary_t& sumActNone = summary[CTrackData::trkpt_t::eActNone];

    if(sumActNone.distance != 0)
    {
        printNoAct = true;
    }
    if(descs.size() > 1 || (descs.size() == 1 && printNoAct))
    {
        printTotal = true;
    }

    // ############### build header ###############
    str += "<tr>";
    str += "<th></th>";
    for(const desc_t *desc : descs)
    {
        str += QString("<th align='right'><img src='%1'/></th>").arg(desc->iconSmall);
    }
    if(printNoAct)
    {
        str += QString("<th align='right'><img src='://icons/16x16/ActNone.png'/></th>");
    }
    if(printTotal)
    {
        str += "<th align='right'>" + tr("Total") + "</th>";
    }
    str += "</tr>";

    // ############### build Distance row ###############
    str += "<tr>";
    str += "<td>" + tr("Distance:") + "</td>";
    distance = 0;
    for(const desc_t *desc : descs)
    {
        const activity_summary_t& s = summary[desc->flag];
        IUnit::self().meter2distance(s.distance, val, unit);
        str += QString("<td align='right'>&nbsp;&nbsp;%1 %2</td>").arg(val).arg(unit);
        distance += s.distance;
    }
    if(printNoAct)
    {
        IUnit::self().meter2distance(sumActNone.distance, val, unit);
        str += QString("<td align='right'>&nbsp;&nbsp;%1 %2</td>").arg(val).arg(unit);
        distance += sumActNone.distance;
    }
    if(printTotal)
    {
        IUnit::self().meter2distance(distance, val, unit);
        str += QString("<td align='right'>&nbsp;&nbsp;%1 %2</td>").arg(val).arg(unit);
    }
    str += "</tr>";

    // ############### build Ascent row ###############
    str += "<tr>";
    str += "<td>" + tr("Ascent:") + "</td>";
    total = 0;
    for(const desc_t *desc : descs)
    {
        const activity_summary_t& s = summary[desc->flag];
        IUnit::self().meter2elevation(s.ascent, val, unit);
        str += QString("<td align='right'>&nbsp;&nbsp;%1%2</td>").arg(val).arg(unit);
        total += s.ascent;
    }
    if(printNoAct)
    {
        IUnit::self().meter2elevation(sumActNone.ascent, val, unit);
        str += QString("<td align='right'>&nbsp;&nbsp;%1%2</td>").arg(val).arg(unit);
        total += sumActNone.ascent;
    }
    if(printTotal)
    {
        IUnit::self().meter2elevation(total, val, unit);
        str += QString("<td align='right'>&nbsp;&nbsp;%1%2</td>").arg(val).arg(unit);
    }
    str += "</tr>";

    // ############### build Descent row ###############
    str += "<tr>";
    str += "<td>" + tr("Descent:") + "</td>";
    total = 0;
    for(const desc_t *desc : descs)
    {
        const activity_summary_t& s = summary[desc->flag];
        IUnit::self().meter2elevation(s.descent, val, unit);
        str += QString("<td align='right'>&nbsp;&nbsp;%1%2</td>").arg(val).arg(unit);
        total += s.descent;
    }
    if(printNoAct)
    {
        IUnit::self().meter2elevation(sumActNone.descent, val, unit);
        str += QString("<td align='right'>&nbsp;&nbsp;%1%2</td>").arg(val).arg(unit);
        total += sumActNone.descent;
    }
    if(printTotal)
    {
        IUnit::self().meter2elevation(total, val, unit);
        str += QString("<td align='right'>&nbsp;&nbsp;%1%2</td>").arg(val).arg(unit);
    }
    str += "</tr>";

    // ############### build Speed Moving row ###############
    str += "<tr>";
    str += "<td>" + tr("Speed Moving:") + "</td>";
    total = 0;
    for(const desc_t *desc : descs)
    {
        const activity_summary_t& s = summary[desc->flag];
        IUnit::self().meter2speed(s.distance/s.ellapsedSecondsMoving, val, unit);
        str += QString("<td align='right'>&nbsp;&nbsp;%1%2</td>").arg(val).arg(unit);
        total += s.ellapsedSecondsMoving;
    }
    if(printNoAct)
    {
        IUnit::self().meter2speed(sumActNone.distance/sumActNone.ellapsedSecondsMoving, val, unit);
        str += QString("<td align='right'>&nbsp;&nbsp;%1%2</td>").arg(val).arg(unit);
        total += sumActNone.ellapsedSecondsMoving;
    }
    if(printTotal)
    {
        IUnit::self().meter2speed(distance/total, val, unit);
        str += QString("<td align='right'>&nbsp;&nbsp;%1%2</td>").arg(val).arg(unit);
    }
    str += "</tr>";

    // ############### build Speed row ###############
    str += "<tr>";
    str += "<td>" + tr("Speed Total:") + "</td>";
    total = 0;
    for(const desc_t *desc : descs)
    {
        const activity_summary_t& s = summary[desc->flag];
        IUnit::self().meter2speed(s.distance/s.ellapsedSeconds, val, unit);
        str += QString("<td align='right'>&nbsp;&nbsp;%1%2</td>").arg(val).arg(unit);
        total += s.ellapsedSeconds;
    }
    if(printNoAct)
    {
        IUnit::self().meter2speed(sumActNone.distance/sumActNone.ellapsedSeconds, val, unit);
        str += QString("<td align='right'>&nbsp;&nbsp;%1%2</td>").arg(val).arg(unit);
        total += sumActNone.ellapsedSeconds;
    }
    if(printTotal)
    {
        IUnit::self().meter2speed(distance/total, val, unit);
        str += QString("<td align='right'>&nbsp;&nbsp;%1%2</td>").arg(val).arg(unit);
    }
    str += "</tr>";

    // ############### build Time Moving row ###############
    str += "<tr>";
    str += "<td>" + tr("Time Moving:") + "</td>";
    total = 0;
    for(const desc_t *desc : descs)
    {
        const activity_summary_t& s = summary[desc->flag];
        IUnit::self().seconds2time(s.ellapsedSecondsMoving, val, unit);
        str += QString("<td align='right'>&nbsp;&nbsp;%1%2</td>").arg(val).arg(unit);
        total += s.ellapsedSecondsMoving;
    }
    if(printNoAct)
    {
        IUnit::self().seconds2time(sumActNone.ellapsedSecondsMoving, val, unit);
        str += QString("<td align='right'>&nbsp;&nbsp;%1%2</td>").arg(val).arg(unit);
        total += sumActNone.ellapsedSecondsMoving;
    }
    if(printTotal)
    {
        IUnit::self().seconds2time(total, val, unit);
        str += QString("<td align='right'>&nbsp;&nbsp;%1%2</td>").arg(val).arg(unit);
    }
    str += "</tr>";

    // ############### build Time Moving row ###############
    str += "<tr>";
    str += "<td>" + tr("Time Total:") + "</td>";
    total = 0;
    for(const desc_t *desc : descs)
    {
        const activity_summary_t& s = summary[desc->flag];
        IUnit::self().seconds2time(s.ellapsedSeconds, val, unit);
        str += QString("<td align='right'>&nbsp;&nbsp;%1%2</td>").arg(val).arg(unit);
        total += s.ellapsedSeconds;
    }
    if(printNoAct)
    {
        IUnit::self().seconds2time(sumActNone.ellapsedSeconds, val, unit);
        str += QString("<td align='right'>&nbsp;&nbsp;%1%2</td>").arg(val).arg(unit);
        total += sumActNone.ellapsedSeconds;
    }
    if(printTotal)
    {
        IUnit::self().seconds2time(total, val, unit);
        str += QString("<td align='right'>&nbsp;&nbsp;%1%2</td>").arg(val).arg(unit);
    }
    str += "</tr>";

    str += "</table>";
}

void CActivityTrk::sumUp(QMap<quint32, activity_summary_t> &summary) const
{
    for(quint32 flag : activitySummary.keys())
    {
        const activity_summary_t &src = activitySummary[flag];
        activity_summary_t       &dst = summary[flag];

        dst.distance += src.distance;
        dst.ascent   += src.ascent;
        dst.descent  += src.descent;
        dst.ellapsedSeconds += src.ellapsedSeconds;
        dst.ellapsedSecondsMoving += src.ellapsedSecondsMoving;
    }
}

const CActivityTrk::desc_t& CActivityTrk::getDescriptor(quint32 flag)
{
    for(const desc_t &desc : actDescriptor)
    {
        if(desc.flag == flag)
        {
            return desc;
        }
    }

    return dummyDesc;
}

void CActivityTrk::setColor(quint32 flag, const QString& color)
{
    for(desc_t &desc : actDescriptor)
    {
        if(desc.flag == flag)
        {
            desc.color = QColor(color);
            return;
        }
    }
}

void CActivityTrk::getActivityNames(QStringList& names) const
{
    for(const desc_t &desc : actDescriptor)
    {
        if((allFlags & desc.flag) != 0)
        {
            names << desc.name;
        }
    }
}

qint32 CActivityTrk::getActivityCount() const
{
    qint32 cnt = 0;
    for(const desc_t &desc : actDescriptor)
    {
        if((allFlags & desc.flag) != 0)
        {
            ++cnt;
        }
    }

    const activity_summary_t& sumActNone = activitySummary[CTrackData::trkpt_t::eActNone];

    if(sumActNone.distance != 0)
    {
        cnt++;
    }


    return cnt;
}