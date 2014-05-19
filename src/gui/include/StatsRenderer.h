#ifndef STATSRENDERER_H /* -*- C++ -*- */
#define STATSRENDERER_H

#include "AbstractButton.h"
#include "SDL/SDL_ttf.h"
#include "Grid.h"
#include "Utils.h"
#include "itype.h"
#include "Point.h"
#include "Fonts.h"
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

namespace MFM {

  template <class GC>
  class StatsRenderer
  {
    // Extract short type names
    typedef typename GC::CORE_CONFIG CC;

  public:

    class CapturableStatistic {
    public:
      virtual const char * GetLabel() const = 0;
      virtual s32 GetDecimalPlaces() const = 0;
      virtual double GetValue(bool endOfEpoch) const = 0;
      virtual ~CapturableStatistic() { }
    };

    class ElementCount : public CapturableStatistic {
      const Element<CC> * m_element;
      const Grid<GC> * m_grid;
    public:
      ElementCount() : m_element(0), m_grid(0) { }

      void Set(const Grid<GC> * grid, const Element<CC> * elt) {
        m_element = elt;
        m_grid = grid;
      }

      virtual const char * GetLabel() const {
        if (m_element)
          return m_element->GetUUID().GetLabel();
        return "<unset>";
      }
      virtual s32 GetDecimalPlaces() const {
        return 0;
      }
      virtual double GetValue(bool) const {
        if (!m_element || !m_grid)
          return -1;
        u32 type = m_element->GetType();
        return (double) m_grid->GetAtomCount(type);
      }
    };

    class ElementDataSlotSum : public CapturableStatistic {
      Grid<GC> * m_grid;
      const char * m_label;
      u32 m_elementType;
      u32 m_slot;
      u32 m_outOfSlots;
      bool m_resetOnRead;
    public:
      ElementDataSlotSum() : m_grid(0), m_label(0), m_elementType(0), m_slot(0), m_outOfSlots(0), m_resetOnRead(false) { }

      void Set(Grid<GC> & grid, const char * label, u32 elementType, u32 slot, u32 outOfSlots, bool resetOnRead) {
        m_grid = &grid;
        m_label = label;
        m_elementType = elementType;
        m_slot = slot;
        m_outOfSlots = outOfSlots;
        m_resetOnRead = resetOnRead;
      }

      virtual const char * GetLabel() const {
        if (m_label)
          return m_label;
        return "<unset>";
      }
      virtual s32 GetDecimalPlaces() const {
        return 0;
      }
      virtual double GetValue(bool endOfEpoch) const {
        if (!m_grid)
          return -1;
        if (m_slot >= m_outOfSlots)
          return -1;

        u64 sum = 0;
        for (typename Grid<GC>::iterator_type i = m_grid->begin(); i != m_grid->end(); ++i) {
          Tile<CC> * t = *i;
          ElementTable<CC> & et = t->GetElementTable();
          u64 * eds = et.GetElementDataSlotsFromType(m_elementType,m_outOfSlots);
          if (!eds) continue;
          sum += eds[m_slot];
          if (endOfEpoch && m_resetOnRead)
            eds[m_slot] = 0;
        }

        return (double) sum;
      }
    };

  private:
    UPoint m_dimensions;
    SPoint m_drawPoint;

    TTF_Font* m_drawFont;

    static const u32 MAX_TYPES = 16;
    const CapturableStatistic *(m_capStats[MAX_TYPES]);
    u32 m_capStatsInUse;

    ElementCount m_displayElements[MAX_TYPES];
    u32 m_displayElementsInUse;

    bool m_displayAER;

    static const u32 MAX_BUTTONS = 16;
    AbstractButton* m_buttons[MAX_BUTTONS];
    u32 m_registeredButtons;

  public:
    StatsRenderer() : m_drawFont(0), m_capStatsInUse(0), m_displayElementsInUse(0), m_displayAER(false),
		      m_registeredButtons(0)
    {}

    void ClearButtons()
    {
      m_registeredButtons = 0;
    }

    void ReassignButtonLocations()
    {
      SPoint loc(4, m_capStatsInUse * 40);
      if(m_displayAER)
      {
	loc.SetY(loc.GetY() + 100);
      }
      for(u32 i = 0; i < m_registeredButtons; i++)
      {
	m_buttons[i]->SetLocation(loc);
	loc.SetY(loc.GetY() + 40);
      }
    }

    void AddButton(AbstractButton* b)
    {
      if (m_registeredButtons >= MAX_BUTTONS)
        FAIL(OUT_OF_ROOM);

      SPoint dimensions(268, 32);
      SPoint location(4, (m_capStatsInUse + m_registeredButtons) * 40);
      b->SetDimensions(dimensions);
      b->SetLocation(location);

      m_buttons[m_registeredButtons++] = b;
    }

    void HandleClick(SPoint& clickPt)
    {
      for(u32 i = 0; i < m_registeredButtons; i++)
      {
	if(m_buttons[i]->Contains(clickPt))
	{
	  m_buttons[i]->OnClick();
	  break;
	}
      }
    }

    /*
    void translateHome(char* buffer)
    {
      if(*buffer == '~')
      {
        const u32 SIZE = 1024;
	char trans[SIZE];
	sprintf(trans, "%s%s",
		getenv("HOME"),
		buffer + 1);
	sprintf(buffer, "%s", trans);
      }
    }
    */

    void OnceOnly(Fonts & fonts) {
      m_drawFont = fonts.GetDefaultFont(30);
    }

    bool GetDisplayAER() const { return m_displayAER; }

    void SetDisplayAER(bool displayAER)
    {
      m_displayAER = displayAER;
      ReassignButtonLocations();
    }

    bool DisplayCapturableStats(const CapturableStatistic * cs) {
      if (m_capStatsInUse >= MAX_TYPES) return false;
      u32 index = m_capStatsInUse++;
      m_capStats[index] = cs;
      return true;
    }

    bool DisplayStatsForElement(const Grid<GC>  & grd, const Element<CC> & elt) {
      if (m_displayElementsInUse >= MAX_TYPES) return false;
      u32 index = m_displayElementsInUse++;
      m_displayElements[index].Set(&grd,&elt);
      return DisplayCapturableStats(&m_displayElements[index]);
    }

    ~StatsRenderer()
    {
      TTF_CloseFont(m_drawFont);
    }

    /*
    void SetDestination(Panel* dest)
    {
      m_dest = dest;
    }
    */

    void SetDrawPoint(Point<s32> drawPoint)
    {
      m_drawPoint = drawPoint;
    }

    void SetDimensions(Point<u32> dimensions)
    {
      m_dimensions = dimensions;
    }

    void RenderGridStatistics(Drawing & drawing, Grid<GC>& grid, double aeps, double aer, u32 AEPSperFrame, double overhead, bool doResets);

    void WriteRegisteredCounts(ByteSink & fp, bool writeHeader, Grid<GC>& grid, double aeps, double aer, u32 AEPSperFrame, double overhead, bool doResets);
  };
} /* namespace MFM */
#include "StatsRenderer.tcc"

#endif /*STATSRENDERER_H*/

