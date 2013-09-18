#ifndef __RESOURCEEDITORQT__HEIGHTMAPEDITORPANEL__
#define __RESOURCEEDITORQT__HEIGHTMAPEDITORPANEL__

#include "LandscapeEditorBasePanel.h"
#include "DAVAEngine.h"
#include "Qt/Scene/System/HeightmapEditorSystem.h"

using namespace DAVA;

class QComboBox;
class QRadioButton;
class QCheckBox;
class QLineEdit;
class SliderWidget;

class HeightmapEditorPanel: public LandscapeEditorBasePanel
{
	Q_OBJECT

public:
	static const int DEF_BRUSH_MIN_SIZE = 3;
	static const int DEF_BRUSH_MAX_SIZE = 40;
	static const int DEF_STRENGTH_MAX_VALUE = 30;
	static const int DEF_AVERAGE_STRENGTH_MIN_VALUE = 0;
	static const int DEF_AVERAGE_STRENGTH_MAX_VALUE = 60;

	explicit HeightmapEditorPanel(QWidget* parent = 0);
	~HeightmapEditorPanel();

private slots:
	void SetDropperHeight(SceneEditor2* scene, double height);
	void HeightUpdatedManually();

	void SetBrushSize(int brushSize);
	void SetToolImage(int toolImage);
	void SetRelativeDrawing();
	void SetAverageDrawing();
	void SetAbsoluteDrawing();
	void SetAbsDropDrawing();
	void SetDropper();
	void SetHeightmapCopyPaste();
	void SetStrength(int strength);
	void SetAverageStrength(int averageStrength);
	void SetCopyPasteHeightmap(int state);
	void SetCopyPasteTilemask(int state);

	void IncreaseBrushSize();
	void DecreaseBrushSize();
	void IncreaseBrushSizeLarge();
	void DecreaseBrushSizeLarge();

	void IncreaseStrength();
	void DecreaseStrength();
	void IncreaseStrengthLarge();
	void DecreaseStrengthLarge();

	void IncreaseAvgStrength();
	void DecreaseAvgStrength();
	void IncreaseAvgStrengthLarge();
	void DecreaseAvgStrengthLarge();

	void PrevTool();
	void NextTool();

	void ShortcutSetCopyPasteHeightmap();
	void ShortcutSetCopyPasteTilemask();

protected:
	virtual bool GetEditorEnabled();

	virtual void OnEditorEnabled();

	virtual void SetWidgetsState(bool enabled);
	virtual void BlockAllSignals(bool block);
	
	virtual void InitUI();
	virtual void ConnectToSignals();
	
	virtual void StoreState();
	virtual void RestoreState();

	virtual void ConnectToShortcuts();
	virtual void DisconnectFromShortcuts();

private:
	SliderWidget* sliderWidgetBrushSize;
	SliderWidget* sliderWidgetStrength;
	SliderWidget* sliderWidgetAverageStrength;
	QComboBox* comboBrushImage;
	QRadioButton* radioCopyPaste;
	QRadioButton* radioAbsDrop;
	QRadioButton* radioAbsolute;
	QRadioButton* radioAverage;
	QRadioButton* radioDropper;
	QRadioButton* radioRelative;
	QCheckBox* checkboxHeightmap;
	QCheckBox* checkboxTilemask;
	QLineEdit* editHeight;

	void InitBrushImages();
	void UpdateRadioState(HeightmapEditorSystem::eHeightmapDrawType type);
	void SetDrawingType(HeightmapEditorSystem::eHeightmapDrawType type);

	float32 GetBrushScaleCoef();
	int32 BrushSizeUIToSystem(int32 uiValue);
	int32 BrushSizeSystemToUI(int32 systemValue);

	float32 StrengthUIToSystem(int32 uiValue);
	int32 StrengthSystemToUI(float32 systemValue);

	float32 AverageStrengthUIToSystem(int32 uiValue);
	int32 AverageStrengthSystemToUI(float32 systemValue);
};

#endif /* defined(__RESOURCEEDITORQT__HEIGHTMAPEDITORPANEL__) */
