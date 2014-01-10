/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#ifndef __qMRMLSubjectHierarchyTreeView_h
#define __qMRMLSubjectHierarchyTreeView_h

// qMRML includes
#include "qMRMLTreeView.h"

// SubjectHierarchy includes
#include "qSlicerSubjectHierarchyModuleWidgetsExport.h"

class vtkMRMLSubjectHierarchyNode;
class qMRMLSubjectHierarchyTreeViewPrivate;
class qMRMLSceneSubjectHierarchyModel;
class vtkSlicerSubjectHierarchyModuleLogic;

/// \ingroup Slicer_QtModules_SubjectHierarchy
class Q_SLICER_MODULE_SUBJECTHIERARCHY_WIDGETS_EXPORT qMRMLSubjectHierarchyTreeView : public qMRMLTreeView
{
  Q_OBJECT

public:
  typedef qMRMLTreeView Superclass;
  qMRMLSubjectHierarchyTreeView(QWidget *parent=0);
  virtual ~qMRMLSubjectHierarchyTreeView();

protected:
  /// Toggle visibility
  virtual void toggleVisibility(const QModelIndex& index);

  /// Set checked state of the select plugin actions in the node context menu for a specific node
  void updatePluginActionCheckedStates(vtkMRMLSubjectHierarchyNode* currentSubjectHierarchyNode);

  /// Populate context menu for current node
  void populateContextMenuForCurrentNode();

  /// Handle mouse press event (facilitates timely update of context menu)
  virtual void mousePressEvent(QMouseEvent* event);

public slots:
  /// Handle current node change. Invokes context menu population.
  void onCurrentNodeChanged(vtkMRMLNode* newCurrentNode);

  /// Handle node creation by plugin (expands branch etc.)
  void expandNode(vtkMRMLSubjectHierarchyNode* node);

  /// Handle manual selection of a plugin as the new owner of a subject hierarchy node
  void selectPluginForCurrentNode();

protected:
  QScopedPointer<qMRMLSubjectHierarchyTreeViewPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSubjectHierarchyTreeView);
  Q_DISABLE_COPY(qMRMLSubjectHierarchyTreeView);
};

#endif