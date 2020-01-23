#ifndef _ART_PROVIDER_H_
#define _ART_PROVIDER_H_

#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/filename.h>

class ArtProvider : public wxArtProvider
{
public:
    ArtProvider(const wxString &dir) : wxArtProvider()
    {
        m_logo.LoadFile(wxFileName(dir, "logo").GetFullPath(), wxBITMAP_TYPE_PNG_RESOURCE);
        m_checkmark.LoadFile(wxFileName(dir, "checkmark").GetFullPath(), wxBITMAP_TYPE_PNG_RESOURCE);
        m_cross.LoadFile(wxFileName(dir, "cross").GetFullPath(), wxBITMAP_TYPE_PNG_RESOURCE);
    }

protected:
    wxBitmap m_logo;
    wxBitmap m_checkmark;
    wxBitmap m_cross;

    virtual wxBitmap CreateBitmap(const wxArtID &id, const wxArtClient &client, const wxSize &size)
    {
        if (id == "logo") {
            return m_logo;
        } else if (id == "checkmark") {
            return m_checkmark;
        } else if (id == "cross") {
            return m_cross;
        }
        return m_logo;
    }
};

#endif
