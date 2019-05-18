#ifndef CBDIFFCONFIGPANEL_H
#define CBDIFFCONFIGPANEL_H

//(*Headers(cbDiffConfigPanel)
#include <wx/panel.h>
class wxBoxSizer;
class wxButton;
class wxChoice;
class wxRadioBox;
class wxSlider;
class wxStaticBoxSizer;
class wxStaticText;
//*)

#include <configurationpanel.h>

class cbDiffConfigPanel: public cbConfigurationPanel
{
	public:

		cbDiffConfigPanel(wxWindow *parent);
		virtual ~cbDiffConfigPanel();

        virtual wxString GetTitle() const;
        /// @return the panel's bitmap base name. You must
        /// supply two bitmaps: \<basename\>.png and \<basename\>-off.png...
        virtual wxString GetBitmapBaseName() const;
        /// Called when the user chooses to apply the configuration.
        virtual void OnApply();
        /// Called when the user chooses to cancel the configuration.
        virtual void OnCancel();
	private:

		//(*Declarations(cbDiffConfigPanel)
		wxButton* BColAdd;
		wxButton* BColCar;
		wxButton* BColRem;
		wxChoice* CHCaret;
		wxRadioBox* RBViewing;
		wxSlider* SLAddAlpha;
		wxSlider* SLCarAlpha;
		wxSlider* SLRemAlpha;
		wxStaticText* StaticText3;
		//*)

		//(*Identifiers(cbDiffConfigPanel)
		static const long ID_BUTTON2;
		static const long ID_STATICTEXT1;
		static const long ID_SLIDER1;
		static const long ID_BUTTON1;
		static const long ID_STATICTEXT2;
		static const long ID_SLIDER2;
		static const long ID_CHOICE1;
		static const long ID_BUTTON3;
		static const long ID_STATICTEXT3;
		static const long ID_SLIDER3;
		static const long ID_RADIOBOX1;
		//*)

		//(*Handlers(cbDiffConfigPanel)
		void OnColAddClick(wxCommandEvent& event);
		void OnColRemClick(wxCommandEvent& event);
		void OnColCarClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
