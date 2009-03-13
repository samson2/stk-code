#ifndef HEADER_WIDGET_HPP
#define HEADER_WIDGET_HPP

#include <irrlicht.h>
#include "ptr_vector.hpp"

using namespace irr;
using namespace gui;

namespace GUIEngine
{
    
    enum WidgetType
    {
        WTYPE_NONE = -1,
        WTYPE_RIBBON,
        WTYPE_SPINNER,
        WTYPE_BUTTON,
        WTYPE_GAUGE,
        WTYPE_ICON_BUTTON,
        WTYPE_CHECKBOX,
        WTYPE_LABEL,
        WTYPE_MODEL,
        WTYPE_SPACER,
        WTYPE_DIV,
        WTYPE_RIBBON_GRID
    };
    
    enum Property
    {
        PROP_ID = 100,
        PROP_PROPORTION,
        PROP_WIDTH,
        PROP_HEIGHT,
        PROP_CHILD_WIDTH,
        PROP_CHILD_HEIGHT,
        PROP_WORD_WRAP,
        PROP_GROW_WITH_TEXT, // yet unused
        PROP_X,
        PROP_Y,
        PROP_LAYOUT,
        PROP_ALIGN,
        PROP_TEXT,
        PROP_ICON,
        PROP_TEXT_ALIGN,
        PROP_MIN_VALUE,
        PROP_MAX_VALUE
    };
    
    class Widget
    {
    protected:
        friend class RibbonWidget;
        friend class Screen;
        friend class SpinnerWidget;
        friend class Skin;
        friend class RibbonGridWidget;
        
        /**
          * Can be used in children to indicate whether a widget is selected or not
          * - in widgets where it makes sense (e.g. ribbon children) and where the
          * irrLicht widget can not directly contain this state
          */
        bool m_selected;
        
        /**
          * Create and add the irrLicht widget(s) associated with this object.
          * Call after Widget was read from XML file and laid out.
          */
        virtual void add() {} 
        
        /**
          * called when left/right keys pressed and focus is on widget. 
          * Returns 'true' if main event handler should be notified of a change.
          * Override in children to be notified of left/right events.
          */
        virtual bool rightPressed() { return false; }
        virtual bool leftPressed() { return false; }
        
        /** used when you set parents - see m_parent explainations below.
            returns whether main event callback should be notified or not */
        virtual bool transmitEvent(Widget* w, std::string& originator) { return true; }
        
        /** used when you set eventSupervisors - see m_parent explainations below
            called when one of a widget's children is hovered.
            Returns 'true' if main event handler should be notified of a change. */
        virtual bool mouseHovered(Widget* child) { return false; }
        
        /** override in children if you need to know when the widget is focused */
        virtual void focused() {}
        
        void readCoords(Widget* parent=NULL);
        
        /**
          * This is set to NULL by default; set to something else in a child to mean
          * that events happening on the said child should not go straight into the
          * event handler. Instead, they will first be passed to m_parent->transmitEvent,
          * which is usually the parent analysing events from its children.
          * This is especially useful with logical widgets built with more than
          * one irrlicht widgets (e.g. Spinner, Ribbon)
          */
        Widget* m_parent;
        
        static bool convertToCoord(std::string& x, int* absolute, int* percentage);
    public:
        virtual ~Widget() {}
        
        /**
          * If this widget has any children, they go here. Children can be either
          * specified in the XML file (e.g. Ribbon or Div children), or can also
          * be created automatically for logical widgets built with more than
          * one irrlicht widgets (e.g. Spinner)
          */
        ptr_vector<Widget> m_children;
        
        /** Type of this widget */
        WidgetType m_type;
        
        /** coordinates of the widget */
        int x, y, w, h;
        
        /** numerical ID used by irrLicht to identify this widget
          * (not the same as the string identificator specified in the XML file)
          */
        int id;
        
        /**
          * IrrLicht widget created to represent this object.
          */
        IGUIElement* m_element;
        
        /** A map that holds values for all specified widget properties (in the XML file)*/
        std::map<Property, std::string> m_properties;
        
        
        Widget();

        static void resetIDCounters();
        
        bool isSelected() const { return m_selected; }

    };
    
    class ButtonWidget : public Widget
    {
        void add();
    public:
        virtual ~ButtonWidget() {}
    };
    
    class LabelWidget : public Widget
    {
        void add();
    public:
        virtual ~LabelWidget() {}
    };
    
    class CheckBoxWidget : public Widget
    {
        void add();
    public:
        virtual ~CheckBoxWidget() {}
    };
    

    class GaugeWidget : public Widget
    {
        void add();
    public:
        virtual ~GaugeWidget() {}
    };
    
    class SpinnerWidget : public Widget
    {
        int m_value, m_min, m_max;
        std::vector<std::string> m_labels;
        bool m_graphical;
        
        bool transmitEvent(Widget* w, std::string& originator);
        bool rightPressed();
        bool leftPressed();
        void add();
    public:
        virtual ~SpinnerWidget() {}
        void setValue(const int new_value);
        void addLabel(std::string label);
    };
    
    class IconButtonWidget : public Widget
    {
        bool clickable;
        void add();
    public:
        virtual ~IconButtonWidget() {}
        IconButtonWidget(const bool clickable=true);
    };
    
    enum RibbonType
    {
        RIBBON_COMBO, /* select one item out of many, like in a combo box */
        RIBBON_TOOLBAR, /* a row of individual buttons */
        RIBBON_TABS /* a tab bar */
    };
    
    class RibbonWidget : public Widget
    {
        friend class RibbonGridWidget;
        friend class Screen;
        
        int m_selection;
        RibbonType m_ribbon_type;
        
        void add();
        
        bool rightPressed();
        bool leftPressed();
        bool mouseHovered(Widget* child);
        
        void updateSelection();
        bool transmitEvent(Widget* w, std::string& originator);
        void focused();
    public:
        virtual ~RibbonWidget() {}
        
        int getSelection() const { return m_selection; }
        RibbonType getRibbonType() const { return m_ribbon_type; }
        const std::string& getSelectionName() { return m_children[m_selection].m_properties[PROP_ID]; }
        
        RibbonWidget(const RibbonType type=RIBBON_COMBO);
    };
    
    struct ItemDescription
    {
        std::string m_user_name;
        std::string m_code_name;
        std::string m_sshot_file;
    };
    
    class RibbonGridWidget : public Widget
    {
        friend class RibbonWidget;
        
        virtual ~RibbonGridWidget() {}
        
        /* reference pointers only, the actual instances are owned by m_children */
        ptr_vector<RibbonWidget, REF> m_rows;
        
        std::vector<ItemDescription> m_items;
        IGUIStaticText* m_label;
        RibbonWidget* getSelectedRibbon() const;
        RibbonWidget* getRowContaining(Widget* w) const;
        
        void updateLabel(RibbonWidget* from_this_ribbon=NULL);
        
        void propagateSelection();
        void focused();
        
        bool transmitEvent(Widget* w, std::string& originator);
        
        void scroll(const int x_delta);
        
        int m_scroll_offset;
        int m_needed_cols;
        int m_col_amount;
        
        bool m_has_label;
        
        /* reference pointers only, the actual instances are owned by m_children */
        Widget* m_left_widget;
        Widget* m_right_widget;
    public:
        RibbonGridWidget();
        
        void add();
        bool rightPressed();
        bool leftPressed();
        
        void addItem( std::string user_name, std::string code_name, std::string image_file );
        void updateItemDisplay();
        
        bool mouseHovered(Widget* child);
        
        const std::string& getSelectionName();
    };

}
#endif