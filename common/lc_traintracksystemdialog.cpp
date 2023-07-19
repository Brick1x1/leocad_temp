#include "lc_global.h"
#include "lc_traintracksystemdialog.h"
#include "ui_lc_traintracksystemdialog.h"    		        
#include "lc_viewwidget.h"
#include "minifig.h"
#include "traintracksystem.h"
#include "lc_application.h"
#include "lc_mainwindow.h"
#include "pieceinf.h"
#include "lc_library.h"
#include "lc_view.h"
#include "lc_model.h"
#include "piece.h"
#include "camera.h"
#include "lc_partselectionwidget.h"
#include <iostream>

lcTrainTrackSystemDialog::lcTrainTrackSystemDialog(QWidget* Parent)
	: QDialog(Parent), ui(new Ui::lcTrainTrackSystemDialog)
{

	gMainWindow->SetTool(lcTool::Pan);

	editor_state = LC_TTS_STATE_INSERT;

	ui->setupUi(this);

	QGridLayout* PreviewLayout = new QGridLayout(ui->trainTrackSystemFrame);
	PreviewLayout->setContentsMargins(0, 0, 0, 0);

	//mMinifigWizard = new MinifigWizard();
	mTrainTrackSystem = new TrainTrackSystem();
	SetExampleTrainTrackSystem();

	mView = new lcView(lcViewType::View, mTrainTrackSystem->GetModel());

	lcViewWidget* ViewWidget = new lcViewWidget(nullptr, mView);
	ViewWidget->setMinimumWidth(1600);

	connect(ViewWidget, &lcViewWidget::mousePressOnPiece, this, &lcTrainTrackSystemDialog::mousePressOnPiece);	

	connect(ViewWidget, &lcViewWidget::mouseRelease, this, &lcTrainTrackSystemDialog::mouseRelease);


	PreviewLayout->addWidget(ViewWidget);

	mView->MakeCurrent();
	//mMinifigWizard->LoadDefault();


	QGridLayout* PartSelector = new QGridLayout(ui->partSelector);
	PartSelector->setContentsMargins(0, 0, 0, 0);

	mPartsWidget = new lcPartSelectionListView(nullptr, nullptr);
	PartSelector->addWidget(mPartsWidget);
	
	mPartsWidget->SetParts(mTrainTrackSystem->getTrackTypes());

	//mMinifigWizard->Calculate();
	mView->GetCamera()->SetViewpoint(lcVector3(0.0f, 0.0f, 90.0f));
	mView->ZoomExtents();
}


lcTrainTrackSystemDialog::~lcTrainTrackSystemDialog()
{
	delete mTrainTrackSystem;
	delete ui;
}


void lcTrainTrackSystemDialog::mousePressOnPiece(lcPiece* pieceClickedOn) {
	
	if(pieceClickedOn != nullptr) {
		
		printf("Piece clicked on (trainstracksystemdialog): %s\n",pieceClickedOn->mPieceInfo->mFileName);

		if(mTrainTrackSystem->findTrackTypeByString(pieceClickedOn->mPieceInfo->mFileName) == -1) {
			
			PieceInfo* piecePartlistInf = mPartsWidget->GetCurrentPart();

			insertTrackType = mTrainTrackSystem->findTrackTypeByString(piecePartlistInf->mFileName);

			printf("Piece selected in part selector (trainstracksystemdialog): %s, %i\n",piecePartlistInf->mFileName,insertTrackType);

			if(selectedFromTrackSection == nullptr) {
				
				lcArray<TrainTrackSection*> trackSections = mTrainTrackSystem->GetTrackSections();
				int found_con_no = 0;		
				//TrainTrackSection* foundTrackSection = nullptr;
				
				for(TrainTrackSection* trackSection : trackSections) {									
					found_con_no = trackSection->FindConnectionNumber(pieceClickedOn->GetRotationCenter());
					//printf("Find piece with location, x: %f, y: %f, z: %f, found connection number: %i\n",lcRotCent.x, lcRotCent.y, lcRotCent.z,found_con_no);					
					if(found_con_no != -1) {
						selectedFromTrackSection = trackSection;
						selectedFromConnectionNo = found_con_no;
						break;
					}
				}
			}
		
			if(selectedFromTrackSection != nullptr) {

				selectedToConnectionNo = 0;
				newSectionInserted = true;

				mTrainTrackSystem->addTrackSection(selectedFromTrackSection, selectedFromConnectionNo, insertTrackType, selectedToConnectionNo);

				mTrainTrackSystem->SetModel();

				mView->Redraw();

			}
		}
		else {

			//Track section is clicke on, delete the track section which is clicked on

			printf("Delete piece clicke on (trainstracksystemdialog): %s, %i\n\n",pieceClickedOn->mPieceInfo->mFileName,insertTrackType);

			if(mTrainTrackSystem->deleteTrackSection(pieceClickedOn) == true) {
				mTrainTrackSystem->SetModel();
				mView->Redraw();
			}


		}

	}


}

void lcTrainTrackSystemDialog::mouseRelease() {
	printf("Mouse released (trainstracksystemdialog)\n");
	selectedFromTrackSection = nullptr;
	newSectionInserted = false;
}


/*bool lcTrainTrackSystemDialog::eventFilter(QObject* Object, QEvent* Event) {
	printf("mouse button pressed!\n");	
	return false;
}*/

void lcTrainTrackSystemDialog::keyPressEvent(QKeyEvent *event) {


	if(selectedFromTrackSection == nullptr) {

		lcModel* activeModel = mView->GetActiveModel();

		const lcArray<lcPiece*>& Pieces = activeModel->GetPieces();

		const lcPiece* pieceFocus = nullptr;
		for (const lcPiece* Piece : Pieces)	{
			if (Piece->IsFocused()) {
				//printf("FOCUSED: piece name: %s\n",Piece->GetName().toLatin1().data());
				pieceFocus = Piece;
				break;
			}
		}

		if(pieceFocus != nullptr) {

			lcArray<TrainTrackSection*> trackSections = mTrainTrackSystem->GetTrackSections();
			int found_con_no = 0;		
			//TrainTrackSection* foundTrackSection = nullptr;
			
			for(TrainTrackSection* trackSection : trackSections) {									
				found_con_no = trackSection->FindConnectionNumber(pieceFocus->GetRotationCenter());
				//printf("Find piece with location, x: %f, y: %f, z: %f, found connection number: %i\n",lcRotCent.x, lcRotCent.y, lcRotCent.z,found_con_no);					
				if(found_con_no != -1) {
					selectedFromTrackSection = trackSection;
					selectedFromConnectionNo = found_con_no;
					break;
				}
			}
		}	
	}
		
	if(selectedFromTrackSection != nullptr) {

		//printf("Connection found, Track type selected: %s, connectio number: %i\n",foundTrackSection->trackType->pieceInfo->m_strDescription, found_con_no);		
		bool updateSection = true;
		switch(event->key()) {
			case Qt::Key_S: {
				insertTrackType = LC_TTS_STRAIGHT;
				selectedToConnectionNo = 0;
				break;
			}
			case Qt::Key_C: {
				insertTrackType = LC_TTS_CURVED;
				selectedToConnectionNo = 0;
				break;
			}
			case Qt::Key_X: {
				insertTrackType = LC_TTS_CROSS;
				selectedToConnectionNo = 0;
				break;
			}
			case Qt::Key_L: {
				insertTrackType = LC_TTS_LEFT_BRANCH;
				selectedToConnectionNo = 0;
				break;
			}
			case Qt::Key_R: {
				insertTrackType = LC_TTS_RIGHT_BRANCH;
				selectedToConnectionNo = 0;
				break;
			}
			case Qt::Key_Right: {
				if(newSectionInserted == true) {
					//selectedToConnectionNo
					lcArray<TrainTrackSection*> trackSections = mTrainTrackSystem->GetTrackSections();
					TrainTrackSection* newTrackSection = trackSections[trackSections.GetSize() - 1];
					
					selectedToConnectionNo++;
					if(selectedToConnectionNo >= newTrackSection->GetNoOfConnections())
						selectedToConnectionNo = 0;										
				}
				break;
			}
			case Qt::Key_Insert: {				
				selectedFromTrackSection = nullptr;
				selectedFromConnectionNo = -1;
				newSectionInserted = false;
				insertTrackType = LC_TTS_STRAIGHT;
				selectedToConnectionNo = 0;
				updateSection = false;	
				break;		
			}

			default: {
				updateSection = false;
			}
		}			

		if(updateSection == true) {

			if(newSectionInserted == true) {
				//printf("remove previous set track section\n");
				lcArray<TrainTrackSection*>* trackSections = mTrainTrackSystem->GetTrackSectionsPtr();
				trackSections->RemoveIndex(trackSections->GetSize() - 1);
			}

			mTrainTrackSystem->addTrackSection(selectedFromTrackSection, selectedFromConnectionNo, insertTrackType, selectedToConnectionNo);

			mTrainTrackSystem->SetModel();

			mView->Redraw();
			newSectionInserted = true;
			//editor_state = LC_TTS_STATE_ROTATE;
		}
	}



}

void lcTrainTrackSystemDialog::SetExampleTrainTrackSystem() 
{
	int colorIndex = 8;
	int mCurrentStep = 1;
	mTrainTrackSystem->setColorIndex(colorIndex);
	mTrainTrackSystem->setCurrentStep(mCurrentStep);
	mTrainTrackSystem->addFirstTrackSection(lcMatrix44Identity(),LC_TTS_STRAIGHT);   //Idx: 0
	/*mTrainTrackSystem->addTrackSection(0, 0, LC_TTS_CURVED      , 0); 				//Idx: 1
	mTrainTrackSystem->addTrackSection(1, 1, LC_TTS_CROSS       , 0); 				//Idx: 2
	mTrainTrackSystem->addTrackSection(2, 3, LC_TTS_CURVED      , 0); 				//Idx: 3
	mTrainTrackSystem->addTrackSection(2, 2, LC_TTS_LEFT_BRANCH , 2); 				//Idx: 4
	mTrainTrackSystem->addTrackSection(0, 1, LC_TTS_RIGHT_BRANCH, 1); 				//Idx: 5
	mTrainTrackSystem->addTrackSection(5, 2, LC_TTS_CURVED      , 0); 				//Idx: 6
	mTrainTrackSystem->addTrackSection(5, 0, LC_TTS_STRAIGHT    , 0); 				//Idx: 7
	mTrainTrackSystem->addTrackSection(6, 1, LC_TTS_STRAIGHT    , 0); 				//Idx: 8*/
}



















