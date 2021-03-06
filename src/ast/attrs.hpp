#ifndef _AST_ATTRS_HPP_
#define _AST_ATTRS_HPP_


namespace AST {

//
class MetaItem;

class MetaItems
{
public:
    Span    m_span;
    ::std::vector<MetaItem> m_items;

    virtual ~MetaItems();
    MetaItems() {}
    MetaItems(MetaItems&&) = default;
    MetaItems& operator=(MetaItems&&) = default;
    MetaItems(const MetaItems&) = delete;
    MetaItems(Span sp, ::std::vector<MetaItem> items):
        m_span( mv$(sp) ),
        m_items( mv$(items) )
    {
    }

    void push_back(MetaItem i);

    MetaItems clone() const;

    MetaItem* get(const char *name) { return const_cast<MetaItem*>( const_cast<const MetaItems*>(this)->get(name)); }
    const MetaItem* get(const char *name) const;
    bool has(const char *name) const {
        return get(name) != 0;
    }

    friend ::std::ostream& operator<<(::std::ostream& os, const MetaItems& x) {
        return os << "[" << x.m_items << "]";
    }
};


TAGGED_UNION(MetaItemData, None,
    (None, struct {}),
    (String, struct {
        ::std::string   val;
        }),
    (List, struct {
        ::std::vector<MetaItem> sub_items;
        })
    );

class MetaItem
{
    ::std::string   m_name;
    MetaItemData    m_data;
public:
    virtual ~MetaItem();
    MetaItem() {}
    MetaItem(MetaItem&& ) = default;
    MetaItem& operator=(MetaItem&& ) = default;
    MetaItem(::std::string name):
        m_name(name),
        m_data( MetaItemData::make_None({}) )
    {
    }
    MetaItem(::std::string name, ::std::string str_val):
        m_name(name),
        m_data( MetaItemData::make_String({mv$(str_val)}) )
    {
    }
    MetaItem(::std::string name, ::std::vector<MetaItem> items):
        m_name(name),
        m_data( MetaItemData::make_List({mv$(items)}) )
    {
    }

    MetaItem clone() const;

    void mark_used() {}
    const ::std::string& name() const { return m_name; }

    bool has_noarg() const { return m_data.is_None(); }

    bool has_string() const { return m_data.is_String(); }
    const ::std::string& string() const { return m_data.as_String().val; }

    bool has_sub_items() const { return m_data.is_List(); }
    const ::std::vector<MetaItem>& items() const { return m_data.as_List().sub_items; }
          ::std::vector<MetaItem>& items()       { return m_data.as_List().sub_items; }

    friend ::std::ostream& operator<<(::std::ostream& os, const MetaItem& x) {
        os << x.m_name;
        TU_MATCH(MetaItemData, (x.m_data), (e),
        (None,
            ),
        (String,
            os << "=\"" << e.val << "\"";
            ),
        (List,
            os << "(" << e.sub_items << ")";
            )
        )
        return os;
    }
};

}   // namespace AST

#endif

