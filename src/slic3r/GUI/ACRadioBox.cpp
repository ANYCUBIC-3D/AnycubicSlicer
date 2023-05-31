#include "ACRadioBox.hpp"

#include "wxExtensions.hpp"

namespace Slic3r { 
namespace GUI {
ACRadioBox::ACRadioBox(wxWindow *parent)
    : wxBitmapToggleButton(parent, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE), m_on(this, "radio_on", 18), m_off(this, "radio_off", 18)
{
    // SetBackgroundStyle(wxBG_STYLE_TRANSPARENT);
    if (parent) SetBackgroundColour(parent->GetBackgroundColour());
    // Bind(wxEVT_TOGGLEBUTTON, [this](auto& e) { update(); e.Skip(); });
    SetSize(m_on.GetSize());
    SetMinSize(m_on.GetSize());
    update();
}

void ACRadioBox::SetValue(bool value)
{
    wxBitmapToggleButton::SetValue(value);
    update();
}

bool ACRadioBox::GetValue()
{
    return wxBitmapToggleButton::GetValue();
}


void ACRadioBox::Rescale()
{
    //m_on.msw_rescale();
    //m_off.msw_rescale();
    SetSize(m_on.GetSize());
    update();
}

void ACRadioBox::update() { SetBitmap((GetValue() ? m_on : m_off).bmp()); }

}
}

