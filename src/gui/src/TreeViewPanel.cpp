#include "TreeViewPanel.h"

namespace MFM {
  void TreeViewPanel::PaintComponent(Drawing& d)
  {
    d.SetBackground(Panel::GetBackground());
    d.SetForeground(Panel::GetForeground());
    d.FillRect(Rect(SPoint(0, 0), Panel::GetDimensions()));

#if 0
    if(!m_atom || !m_grid)
      {
        d.SetFont(AssetManager::Get(FONT_ASSET_HELPPANEL_SMALL));
        d.SetForeground(Drawing::BLACK);
        const char* message = "No atom selected.";
        d.BlitText(message, UPoint(32, 32), MakeUnsigned(d.GetTextSize(message)));
      }
    else
      {
        const Element<EC>* element = m_grid->LookupElement(m_atom->GetType());
        d.SetForeground(element->DefaultPhysicsColor());
        d.FillCircle(2, 2, ATOM_DRAW_SIZE, ATOM_DRAW_SIZE, ATOM_DRAW_SIZE >> 1);
        d.SetFont(FONT_ASSET_ELEMENT);
        d.SetForeground(Drawing::WHITE);
        d.SetBackground(Drawing::BLACK);

        /* As long as the font is monospaced, we can get the text size
           of any 2-character string for this centering. */
        const UPoint textSize = MakeUnsigned(d.GetTextSize("12"));
        d.BlitBackedTextCentered(element->GetAtomicSymbol(), UPoint(8, 8), textSize);

        d.BlitBackedText(element->GetName(), UPoint(4 + ATOM_DRAW_SIZE, 2),
                         MakeUnsigned(d.GetTextSize(element->GetName())));

        OString64 desc;
        element->AppendDescription(m_atom, desc);
        const char* zstr = desc.GetZString();

        d.SetFont(FONT_ASSET_HELPPANEL_SMALL);
        d.BlitBackedText(zstr, UPoint(4 + ATOM_DRAW_SIZE, 28),
                         MakeUnsigned(d.GetTextSize(zstr)));

        OString64 atomBody;
        AtomSerializer<AC> serializer(*m_atom);
        atomBody.Printf("%@", &serializer);
        zstr = atomBody.GetZString();

        d.SetFont(FONT_ASSET_HELPPANEL_SMALL);
        d.BlitBackedText(zstr, UPoint(4 + ATOM_DRAW_SIZE, ATOM_DRAW_SIZE - 4),
                         MakeUnsigned(d.GetTextSize(zstr)));

        PaintDisplayAtomicControllers(d, *m_atom, element);
      }
#endif
  }


}
