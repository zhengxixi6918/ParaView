/*=========================================================================

   Program: ParaView
   Module:  pqFileNamePropertyWidget.cxx

   Copyright (c) 2005,2006 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2.

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

========================================================================*/
#include "pqFileNamePropertyWidget.h"

#include "pqCoreUtilities.h"
#include "vtkCommand.h"
#include "vtkSMInputFileNameDomain.h"
#include "vtkSMStringVectorProperty.h"
#include "vtkSMUncheckedPropertyHelper.h"

#include "pqCoreUtilities.h"
#include "pqDoubleRangeWidget.h"
#include "pqHighlightablePushButton.h"
#include "pqLineEdit.h"
#include "pqPropertiesPanel.h"
#include "pqListPropertyWidget.h"
#include "pqSignalAdaptors.h"
#include "pqWidgetRangeDomain.h"

#include <QtDebug>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QStyle>
#include <QMenu>
#include <iostream>

//-----------------------------------------------------------------------------
pqFileNamePropertyWidget::pqFileNamePropertyWidget(
  vtkSMProxy* smproxy, vtkSMProperty* smproperty, QWidget* parentObject)
  : pqPropertyWidget(smproxy, parentObject)
{
  this->setChangeAvailableAsChangeFinished(false);

  vtkSMStringVectorProperty *svp = vtkSMStringVectorProperty::SafeDownCast(smproperty);
  if(!svp)
    {
    return;
    }
  
  // find the domain
  vtkSMInputFileNameDomain *defaultDomain = NULL;

  vtkSMDomain *domain = svp->FindDomain("vtkSMInputFileNameDomain");

  if(!domain)
    {
    defaultDomain = vtkSMInputFileNameDomain::New();
    domain = defaultDomain;
    }

  QHBoxLayout *layoutLocal = new QHBoxLayout;
  layoutLocal->setMargin(0);
  layoutLocal->setSpacing(pqPropertiesPanel::suggestedHorizontalSpacing());

  if(vtkSMInputFileNameDomain *fileNameDomain = vtkSMInputFileNameDomain::SafeDownCast(domain))
  {
    QLineEdit* lineEdit = new pqLineEdit(this);
    lineEdit->setObjectName(smproxy->GetPropertyName(smproperty));
    this->addPropertyLink(lineEdit, "text",
                          SIGNAL(textChanged(const QString&)), smproperty);
    this->connect(lineEdit, SIGNAL(textChangedAndEditingFinished()),
                  this, SIGNAL(changeFinished()));
    this->setChangeAvailableAsChangeFinished(false);
    
    layoutLocal->addWidget(lineEdit);
    this->setShowLabel(false);

    PV_DEBUG_PANELS() << "LineEdit for a "
                  << "StringVectorProperty with a InputFileNameDomain ("
                  << pqPropertyWidget::getXMLName(fileNameDomain) << ") ";
  }

  if (svp->FindDomain("vtkSMInputFileNameDomain") != NULL)
    {
    PV_DEBUG_PANELS() << "Adding \"Reset\" button since the domain is dynamically";

    // if this has an vtkSMArrayRangeDomain, add a "reset" button.
    pqHighlightablePushButton* resetButton = new pqHighlightablePushButton(this);
    resetButton->setObjectName("Reset");
    resetButton->setToolTip("Reset using current data values");
    resetButton->setIcon(resetButton->style()->standardIcon(QStyle::SP_BrowserReload));
    // resetButton->setFixedWidth(32);

    pqCoreUtilities::connect(svp, vtkCommand::DomainModifiedEvent,
      this, SIGNAL(highlightResetButton()));
    pqCoreUtilities::connect(svp, vtkCommand::UncheckedPropertyModifiedEvent,
      this, SIGNAL(highlightResetButton()));

    this->connect(resetButton, SIGNAL(clicked()), SLOT(resetButtonClicked()));
    resetButton->connect(this, SIGNAL(highlightResetButton()), SLOT(highlight()));
    resetButton->connect(this, SIGNAL(clearHighlight()), SLOT(clear()));

    layoutLocal->addWidget(resetButton);
    }

  this->setLayout(layoutLocal);

  if (defaultDomain)
    {
    defaultDomain->Delete();
    }
}

//-----------------------------------------------------------------------------
pqFileNamePropertyWidget::~pqFileNamePropertyWidget()
{
}

//-----------------------------------------------------------------------------
void pqFileNamePropertyWidget::resetButtonClicked()
{
  // Now this logic to hardcoded for the Environment Annotation filter (hence the name of this
  // widget).

  vtkSMProxy* smproxy = this->proxy();
  vtkSMProperty* smproperty = this->property();

  const char* fileName = "";
  if (vtkSMInputFileNameDomain* domain = vtkSMInputFileNameDomain::SafeDownCast(
      smproperty->GetDomain("filename")))
    {
    if (domain->GetFileName() != NULL)
      {
      fileName = domain->GetFileName();
      // std::cout << fileName << std::endl;
      }
    }


  vtkSMUncheckedPropertyHelper helper(smproperty);
  if (helper.GetAsString() != fileName)
    {
    vtkSMUncheckedPropertyHelper(smproxy, "FileName").Set(fileName);
    emit this->changeAvailable();
    emit this->changeFinished();
    return;
    }
}
