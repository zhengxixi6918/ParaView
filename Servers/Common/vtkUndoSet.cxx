/*=========================================================================

  Program:   ParaView
  Module:    vtkUndoSet.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkUndoSet.h"

#include "vtkCollection.h"
#include "vtkCollectionIterator.h"
#include "vtkObjectFactory.h"
#include "vtkPVXMLElement.h"
#include "vtkUndoElement.h"

vtkStandardNewMacro(vtkUndoSet);
vtkCxxRevisionMacro(vtkUndoSet, "1.3");
//-----------------------------------------------------------------------------
vtkUndoSet::vtkUndoSet()
{
  this->Collection = vtkCollection::New();
}

//-----------------------------------------------------------------------------
vtkUndoSet::~vtkUndoSet()
{
  this->Collection->Delete();
}

//-----------------------------------------------------------------------------
int vtkUndoSet::AddElement(vtkUndoElement* elem)
{
  int num_elements =  this->Collection->GetNumberOfItems();
  if (elem->GetMergeable() && num_elements > 0)
    {
    vtkUndoElement* prev = vtkUndoElement::SafeDownCast(
      this->Collection->GetItemAsObject(num_elements-1));
    if (prev && prev->GetMergeable())
      {
      if (prev->Merge(elem))
        {
        // merge was successful, return index of the merge.
        return (num_elements-1);
        }
      }
    }

  this->Collection->AddItem(elem);
  return num_elements;
}

//-----------------------------------------------------------------------------
void vtkUndoSet::RemoveElement(int index)
{
  this->Collection->RemoveItem(index);
}

//-----------------------------------------------------------------------------
void vtkUndoSet::RemoveAllElements()
{
  this->Collection->RemoveAllItems();
}

//-----------------------------------------------------------------------------
int vtkUndoSet::GetNumberOfElements()
{
  return this->Collection->GetNumberOfItems();
}

//-----------------------------------------------------------------------------
int vtkUndoSet::Redo()
{
  int max = this->Collection->GetNumberOfItems();
  for (int cc=0; cc <max; cc++)
    {
    vtkUndoElement* elem = vtkUndoElement::SafeDownCast(
      this->Collection->GetItemAsObject(cc));
    if (!elem->Redo())
      {
      // redo failed, undo the half redone operations.
      for (int rr=cc-1; rr >=0; --rr)
        {
        vtkUndoElement* elemU = vtkUndoElement::SafeDownCast(
          this->Collection->GetItemAsObject(rr));
        elemU->Undo();
        }
      return 0;
      }
    }
  return 1;
  
}

//-----------------------------------------------------------------------------
int vtkUndoSet::Undo()
{
  int max = this->Collection->GetNumberOfItems();
  for (int cc=max-1; cc >=0; --cc)
    {
    vtkUndoElement* elem = vtkUndoElement::SafeDownCast(
      this->Collection->GetItemAsObject(cc));
    if (!elem->Undo())
      {
      // undo failed, redo the half undone operations.
      for (int rr=0; rr <cc; ++rr)
        {
        vtkUndoElement* elemR = vtkUndoElement::SafeDownCast(
          this->Collection->GetItemAsObject(rr));
        elemR->Redo();
        }
      return 0;
      }
    }
  return 1;
}

//-----------------------------------------------------------------------------
vtkPVXMLElement* vtkUndoSet::SaveState(vtkPVXMLElement* root)
{

  vtkPVXMLElement* elem = vtkPVXMLElement::New();
  elem->SetName("UndoSet");
  if (root)
    {
    root->AddNestedElement(elem);
    elem->Delete();
    }
  
  int max = this->Collection->GetNumberOfItems();
  for (int cc=0; cc <max; cc++)
    {
    vtkUndoElement* child_elem = vtkUndoElement::SafeDownCast(
      this->Collection->GetItemAsObject(cc));
    child_elem->SaveState(elem);
    }
  return elem;
}

//-----------------------------------------------------------------------------
void vtkUndoSet::LoadState(vtkPVXMLElement* elem)
{
  if (!elem )
    {
    vtkErrorMacro("Element must be specified to load state from.");
    return;
    }
  
  if (!elem->GetName() || strcmp(elem->GetName(), "UndoSet") != 0)
    {
    vtkErrorMacro("Invalid element type. <UndoSet /> required.");
    return;
    }
  vtkWarningMacro("State loading is not supported.");
}

//-----------------------------------------------------------------------------
void vtkUndoSet::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
