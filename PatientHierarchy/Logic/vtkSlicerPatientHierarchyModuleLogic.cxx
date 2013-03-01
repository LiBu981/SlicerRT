/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

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

// PatientHierarchy includes
#include "vtkSlicerPatientHierarchyModuleLogic.h"

// SlicerRT includes
#include "SlicerRtCommon.h"

// MRML includes
#include <vtkMRMLHierarchyNode.h>

// VTK includes
#include <vtkNew.h>

// ITK includes
#include <itkImageRegionIteratorWithIndex.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
const char* vtkSlicerPatientHierarchyModuleLogic::PATIENTHIERARCHY_LEVEL_PATIENT = "Patient";
const char* vtkSlicerPatientHierarchyModuleLogic::PATIENTHIERARCHY_LEVEL_STUDY = "Study";
const char* vtkSlicerPatientHierarchyModuleLogic::PATIENTHIERARCHY_LEVEL_SERIES = "Series";
const char* vtkSlicerPatientHierarchyModuleLogic::PATIENTHIERARCHY_LEVEL_SUBSERIES = "Subseries";

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerPatientHierarchyModuleLogic);

//----------------------------------------------------------------------------
vtkSlicerPatientHierarchyModuleLogic::vtkSlicerPatientHierarchyModuleLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerPatientHierarchyModuleLogic::~vtkSlicerPatientHierarchyModuleLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerPatientHierarchyModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
void vtkSlicerPatientHierarchyModuleLogic::RegisterNodes()
{
}

//---------------------------------------------------------------------------
void vtkSlicerPatientHierarchyModuleLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);

  this->Modified();
}

//---------------------------------------------------------------------------
vtkMRMLHierarchyNode*
vtkSlicerPatientHierarchyModuleLogic::GetPatientHierarchyNodeByUid( vtkMRMLScene *scene, const char* uid )
{
  if (!scene || !uid)
  {
    return NULL;
  }

  std::vector<vtkMRMLNode *> patientHierarchyNodes;
  unsigned int numberOfNodes = scene->GetNodesByClass("vtkMRMLHierarchyNode", patientHierarchyNodes);

  for (unsigned int i=0; i<numberOfNodes; i++)
  {
    vtkMRMLHierarchyNode* node = vtkMRMLHierarchyNode::SafeDownCast(patientHierarchyNodes[i]);
    if (node && vtkSlicerPatientHierarchyModuleLogic::IsPatientHierarchyNode(node))
    {
      const char* nodeUid = node->GetAttribute(SlicerRtCommon::PATIENTHIERARCHY_DICOMUID_ATTRIBUTE_NAME);
      if (nodeUid && !STRCASECMP(uid, nodeUid))
      {
        return node;
      }
    }
  }

  return NULL;
}

//---------------------------------------------------------------------------
void vtkSlicerPatientHierarchyModuleLogic::InsertDicomSeriesInHierarchy(
  vtkMRMLScene *scene, const char* patientId, const char* studyInstanceUid, const char* seriesInstanceUid )
{
  if ( !scene || !patientId || !studyInstanceUid || !seriesInstanceUid )
  {
    return;
  }

  vtkMRMLHierarchyNode* patientNode = NULL;
  vtkMRMLHierarchyNode* studyNode = NULL;
  vtkMRMLHierarchyNode* seriesNode = NULL;

  std::vector<vtkMRMLNode*> patientHierarchyNodes;
  unsigned int numberOfNodes = scene->GetNodesByClass("vtkMRMLHierarchyNode", patientHierarchyNodes);

  // Find referenced nodes
  for (unsigned int i=0; i<numberOfNodes; i++)
  {
    vtkMRMLHierarchyNode *node = vtkMRMLHierarchyNode::SafeDownCast(patientHierarchyNodes[i]);
    if ( node && vtkSlicerPatientHierarchyModuleLogic::IsPatientHierarchyNode(node) )
    {
      const char* nodeUid = node->GetAttribute(SlicerRtCommon::PATIENTHIERARCHY_DICOMUID_ATTRIBUTE_NAME);
      if (!nodeUid)
      {
        vtkWarningWithObjectMacro(scene,
          "Patient hierarchy node '" << node->GetName() << "' does not have a Uid thus invalid!");
        continue;
      }
      if (!STRCASECMP(patientId, nodeUid))
      {
        patientNode = node;
      }
      else if (!STRCASECMP(studyInstanceUid, nodeUid))
      {
        studyNode = node;
      }
      else if (!STRCASECMP(seriesInstanceUid, nodeUid))
      {
        seriesNode = node;
      }
    }
  }

  if (!seriesNode)
  {
    vtkErrorWithObjectMacro(scene,
      "vtkSlicerPatientHierarchyModuleLogic::InsertDicomSeriesInHierarchy: Patient hierarchy node with ID="
      << patientId << " cannot be found!");
    return;
  }

  // Create patient and study nodes if they do not exist yet
  if (!patientNode)
  {
    patientNode = vtkMRMLHierarchyNode::New();
    patientNode->AllowMultipleChildrenOn();
    patientNode->HideFromEditorsOff();
    patientNode->SetAttribute(SlicerRtCommon::PATIENTHIERARCHY_NODE_TYPE_ATTRIBUTE_NAME,
      SlicerRtCommon::PATIENTHIERARCHY_NODE_TYPE_ATTRIBUTE_VALUE);
    patientNode->SetAttribute(SlicerRtCommon::PATIENTHIERARCHY_DICOMLEVEL_ATTRIBUTE_NAME,
      vtkSlicerPatientHierarchyModuleLogic::PATIENTHIERARCHY_LEVEL_PATIENT);
    patientNode->SetAttribute(SlicerRtCommon::PATIENTHIERARCHY_DICOMUID_ATTRIBUTE_NAME,
      patientId);
    scene->AddNode(patientNode);
  }

  if (!studyNode)
  {
    studyNode = vtkMRMLHierarchyNode::New();
    studyNode->AllowMultipleChildrenOn();
    studyNode->HideFromEditorsOff();
    studyNode->SetAttribute(SlicerRtCommon::PATIENTHIERARCHY_NODE_TYPE_ATTRIBUTE_NAME,
      SlicerRtCommon::PATIENTHIERARCHY_NODE_TYPE_ATTRIBUTE_VALUE);
    studyNode->SetAttribute(SlicerRtCommon::PATIENTHIERARCHY_DICOMLEVEL_ATTRIBUTE_NAME,
      vtkSlicerPatientHierarchyModuleLogic::PATIENTHIERARCHY_LEVEL_STUDY);
    studyNode->SetAttribute(SlicerRtCommon::PATIENTHIERARCHY_DICOMUID_ATTRIBUTE_NAME,
      studyInstanceUid);
    studyNode->SetParentNodeID(patientNode->GetID());
    scene->AddNode(studyNode);
  }

  seriesNode->SetParentNodeID(studyNode->GetID());
}

//---------------------------------------------------------------------------
bool vtkSlicerPatientHierarchyModuleLogic::AreNodesInSameBranch(vtkMRMLScene *scene,
                                                                const char* nodeId1, const char* nodeId2,
                                                                const char* lowestCommonLevel/*=NULL*/)
{
  if ( !scene || !nodeId1 || !nodeId2 )
  {
    return false;
  }

  if (lowestCommonLevel < 0)
  {
    vtkErrorWithObjectMacro(scene,
      "vtkSlicerPatientHierarchyModuleLogic::AreNodesInSameBranch: Invalid lowest common level!");
    return false;
  }

  // Get input nodes
  vtkMRMLHierarchyNode* node1 = vtkMRMLHierarchyNode::SafeDownCast(scene->GetNodeByID(nodeId1));
  vtkMRMLHierarchyNode* node2 = vtkMRMLHierarchyNode::SafeDownCast(scene->GetNodeByID(nodeId2));

  // If not hierarchy nodes, check if they have an associated patient hierarchy node
  if (!node1)
  {
    node1 = vtkMRMLHierarchyNode::SafeDownCast(
      vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(scene, nodeId1) );
  }
  if (!node2)
  {
    node2 = vtkMRMLHierarchyNode::SafeDownCast(
      vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(scene, nodeId2) );
  }

  // Check if valid nodes are found
  if ( !node1 || vtkSlicerPatientHierarchyModuleLogic::IsPatientHierarchyNode(node1)
    || !node2 || vtkSlicerPatientHierarchyModuleLogic::IsPatientHierarchyNode(node2) )
  {
    return false;
  }

  // Walk the hierarchy up until we reach the lowest common level
  if (lowestCommonLevel)
  {
    while (true)
    {
      node1 = vtkMRMLHierarchyNode::SafeDownCast(node1->GetParentNode());
      if (!vtkSlicerPatientHierarchyModuleLogic::IsPatientHierarchyNode(node1))
      {
        node1 = NULL;
        vtkWarningWithObjectMacro(scene, "Patient hierarchy node (ID='" << nodeId1 << "') has no ancestor with DICOM level '" << lowestCommonLevel << "'");
        break;
      }
      const char* node1Level = node1->GetAttribute(SlicerRtCommon::PATIENTHIERARCHY_DICOMLEVEL_ATTRIBUTE_NAME);
      if (!node1Level)
      {
        node1 = NULL;
        vtkWarningWithObjectMacro(scene, "Patient hierarchy node (ID='" << nodeId1 << "') has no DICOM level '" << lowestCommonLevel << "'");
        break;
      }
      if (!STRCASECMP(node1Level, lowestCommonLevel))
      {
        break;
      }
    }

    while (true)
    {
      node2 = vtkMRMLHierarchyNode::SafeDownCast(node2->GetParentNode());
      if (!vtkSlicerPatientHierarchyModuleLogic::IsPatientHierarchyNode(node2))
      {
        node2 = NULL;
        vtkWarningWithObjectMacro(scene, "Patient hierarchy node (ID='" << nodeId2 << "') has no ancestor with DICOM level '" << lowestCommonLevel << "'");
        break;
      }
      const char* node2Level = node1->GetAttribute(SlicerRtCommon::PATIENTHIERARCHY_DICOMLEVEL_ATTRIBUTE_NAME);
      if (!node2Level)
      {
        node2 = NULL;
        vtkWarningWithObjectMacro(scene, "Patient hierarchy node (ID='" << nodeId2 << "') has no DICOM level '" << lowestCommonLevel << "'");
        break;
      }
      if (!STRCASECMP(node2Level, lowestCommonLevel))
      {
        break;
      }
    }
  }

  const char* node1Uid = node1->GetAttribute(SlicerRtCommon::PATIENTHIERARCHY_DICOMUID_ATTRIBUTE_NAME);
  const char* node2Uid = node2->GetAttribute(SlicerRtCommon::PATIENTHIERARCHY_DICOMUID_ATTRIBUTE_NAME);
  if ( !node1Uid || !node2Uid )
  {
    vtkErrorWithObjectMacro(scene,
      "vtkSlicerPatientHierarchyModuleLogic::AreNodesInSameBranch: Found ancestor node contains empty UID!");
    return false;
  }

  // Check if the found nodes match
  // (handling the case when two patient hierarchy nodes point to the same DICOM object)
  if ( !strcmp(node1Uid, node2Uid) )
  {
    return true;
  }

  return false;
}

//---------------------------------------------------------------------------
bool vtkSlicerPatientHierarchyModuleLogic::IsPatientHierarchyNode(vtkMRMLNode *node)
{
  bool isPatientHierarchyNode = false;
  if (node->IsA("vtkMRMLHierarchyNode"))
  {
    const char* nodeType = node->GetAttribute(SlicerRtCommon::PATIENTHIERARCHY_NODE_TYPE_ATTRIBUTE_NAME);
    if ( nodeType && !STRCASECMP(nodeType, SlicerRtCommon::PATIENTHIERARCHY_NODE_TYPE_ATTRIBUTE_VALUE) )
    {
      if ( node->GetAttribute(SlicerRtCommon::PATIENTHIERARCHY_DICOMLEVEL_ATTRIBUTE_NAME) )
      {
        isPatientHierarchyNode = true;
      }
    }
  }

  return isPatientHierarchyNode;
}