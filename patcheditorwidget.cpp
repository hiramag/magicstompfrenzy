/****************************************************************************
**
** Copyright (C) 2018 Robert Vetter.
**
** This file is part of the MagicstompFrenzy - an editor for Yamaha Magicstomp
** effect processor
**
** THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
** ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
** IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
** PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
**
** GNU General Public License Usage
** This file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version . The licenses are
** as published by the Free Software Foundation and appearing in the file LICENSE
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**/
#include "patcheditorwidget.h"
#include "patchcommoneditorwidget.h"

#include "magicstomp.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QVariant>

#include "effecteditwidgets/ampmultiwidget.h"
#include "effecteditwidgets/multibanddelaywidget.h"
#include "effecteditwidgets/compressorwidget.h"
#include "effecteditwidgets/basspreamp.h"
#include "effecteditwidgets/hqpitchwidget.h"
#include "effecteditwidgets/dualpitchwidget.h"
#include "effecteditwidgets/acousticmultiwidget.h"
#include "effecteditwidgets/reverbwidget.h"
#include "effecteditwidgets/gatereverbwidget.h"
#include "effecteditwidgets/choruswidget.h"
#include "effecteditwidgets/flangewidget.h"
#include "effecteditwidgets/symphonicwidget.h"
#include "effecteditwidgets/vintageflangewidget.h"
#include "effecteditwidgets/phaserwidget.h"
#include "effecteditwidgets/vintagephaserwidget.h"

PatchEditorWidget::PatchEditorWidget( QWidget *parent)
    : ArrayDataEditWidget( parent),effectEditWidget(nullptr)
{
    mainLayout = new QVBoxLayout();
    mainLayout->addWidget( new PatchCommonEditorWidget());
    mainLayout->addWidget( copyrightLabel = new QLabel(copyrightStr), 16);
    setLayout( mainLayout);
}


void PatchEditorWidget::setDataArray(QByteArray *arr)
{
    qDebug("PatchEditorWidget: New data array");
    ArrayDataEditWidget::setDataArray(arr);
    if(arr == nullptr || arr->size() != PatchTotalLength)
        return;

    copyrightLabel->setText(copyrightStr);
    mainLayout->removeWidget( copyrightLabel);
    if(effectEditWidget != nullptr)
    {
        mainLayout->removeWidget( effectEditWidget);
        delete effectEditWidget;
    }

    EffectTypeId patchType = static_cast<EffectTypeId>(arr->at(PatchType+1)); // only last byte is used in PatchType
    switch(patchType)
    {
    case AcousticMulti:
        mainLayout->addWidget( effectEditWidget = new AcousticMultiWidget(), 8);
        break;
    case EightBandParallelDelay:
    case EightBandSeriesDelay:
    case FourBand2TapModDelay:
    case TwoBand4TapModDelay:
    case EightMultiTapModDelay:
    case TwoBandLong4ShortModDelay:
    case ShortMediumLongModDelay:
        mainLayout->addWidget( effectEditWidget = new MultibandDelayWidget(patchType), 8);
        break;
    case Reverb:
        mainLayout->addWidget( effectEditWidget = new ReverbWidget(), 8);
        break;
    case EarlyRef:
        mainLayout->addWidget( effectEditWidget = new GateReverbWidget(true), 8);
        break;
    case GateReverb:
    case ReverseGate:
        mainLayout->addWidget( effectEditWidget = new GateReverbWidget(), 8);
        break;
    case Chorus:
        mainLayout->addWidget( effectEditWidget = new ChorusWidget(), 8);
        break;
    case Flange:
        mainLayout->addWidget( effectEditWidget = new FlangeWidget(), 8);
        break;
    case VintageFlange:
        mainLayout->addWidget( effectEditWidget = new VintageFlangeWidget(), 8);
        break;
    case Symphonic:
        mainLayout->addWidget( effectEditWidget = new SymphonicWidget(), 8);
        break;
    case Phaser:
        mainLayout->addWidget( effectEditWidget = new PhaserWidget(), 8);
        break;
    case MonoVintagePhaser:
        mainLayout->addWidget( effectEditWidget = new VintagePhaserWidget(), 8);
        break;
    case StereoVintagePhaser:
        mainLayout->addWidget( effectEditWidget = new VintagePhaserWidget(false), 8);
        break;
    case Compressor:
        mainLayout->addWidget( effectEditWidget =
                new CompressorWidget(
                        CompressorWidget::Threshold,
                        CompressorWidget::Ratio,
                        CompressorWidget::Attack,
                        CompressorWidget::Release,
                        CompressorWidget::Knee,
                        CompressorWidget::Gain )
                , 8);
        break;
    case AmpSimulator:
        mainLayout->addWidget( effectEditWidget = new AmpMultiWidget(AmpMultiWidget::AmpSimulatorOnly), 8);
        break;
    case Distortion:
        mainLayout->addWidget( effectEditWidget = new AmpMultiWidget(AmpMultiWidget::DistortionOnly), 8);
        break;
    case AmpMultiChorus:
        mainLayout->addWidget( effectEditWidget = new AmpMultiWidget(AmpMultiWidget::AmpChorus), 8);
        break;
    case AmpMultiFlange:
        mainLayout->addWidget( effectEditWidget = new AmpMultiWidget(AmpMultiWidget::AmpFlange), 8);
        break;
    case AmpMultiPhaser:
        mainLayout->addWidget( effectEditWidget = new AmpMultiWidget(AmpMultiWidget::AmpPhaser), 8);
        break;
    case AmpMultiTremolo:
        mainLayout->addWidget( effectEditWidget = new AmpMultiWidget(AmpMultiWidget::AmpTremolo), 8);
        break;
    case DistorionMultiChorus:
        mainLayout->addWidget( effectEditWidget = new AmpMultiWidget(AmpMultiWidget::DistortionChorus), 8);
        break;
    case DistorionMultiFlange:
        mainLayout->addWidget( effectEditWidget = new AmpMultiWidget(AmpMultiWidget::DistortionFlange), 8);
        break;
    case DistorionMultiPhaser:
        mainLayout->addWidget( effectEditWidget = new AmpMultiWidget(AmpMultiWidget::DistortionPhaser), 8);
        break;
    case DistorionMultiTremolo:
        mainLayout->addWidget( effectEditWidget = new AmpMultiWidget(AmpMultiWidget::DistortionTremolo), 8);
        break;
    case HQPitch:
        mainLayout->addWidget( effectEditWidget = new HQPitchWidget(), 8);
        break;
    case DualPitch:
        mainLayout->addWidget( effectEditWidget = new DualPitchWidget(), 8);
        break;
    case BassPreamp:
        mainLayout->addWidget( effectEditWidget = new BassPreampWidget(), 8);
        break;
    default:
        mainLayout->addWidget(effectEditWidget = new QWidget());
        copyrightLabel->setText(QString("No editor created yet!\n")+copyrightStr);
        break;
    }
    effectEditWidget->setProperty( ArrayDataEditWidget::dataOffsetProperty, QVariant(PatchCommonLength));
    mainLayout->addWidget( copyrightLabel, 16);
    refreshData(0, PatchTotalLength);
}
