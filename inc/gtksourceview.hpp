#include <gtkmm/textview.h>
#include <gtksourceview/gtksourceview.h>

typedef struct _GtkSourceView GtkSourceView;
typedef struct _GtkSourceViewClass GtkSourceViewClass;

namespace Gtk
{
	class SourceView_Class;
}

namespace Gtk
{

class SourceView : public TextView
{
public:
	typedef SourceView CppObjectType;
	typedef SourceView_Class CppClassType;
	typedef GtkSourceView BaseObjectType;
	typedef GtkSourceViewClass BaseClassType;

	virtual ~SourceView();

private:
	friend class SourceView_Class;
	static CppClassType sourceview_class_;

	// noncopyable
	SourceView(const SourceView& other);
	SourceView& operator=(const SourceView& other);

protected:
	explicit SourceView(const Glib::ConstructParams& construct_params);
	explicit SourceView(GtkSourceView* castitem);

public:
	static GType get_type() G_GNUC_CONST;
	static GType get_base_type() G_GNUC_CONST;

	GtkSourceView* gobj()
       		{ return reinterpret_cast<GtkSourceView*>(gobject_); }
	const GtkSourceView* gobj() const
       		{ return reinterpret_cast<GtkSourceView*>(gobject_); }

protected:
	// Default Signal handlers
	// ...
public:
	SourceView();
//	explicit SourceView(const Glib::RefPtr<SourceBuffer>& buffer);

//	Glib::RefPtr<SourceBuffer> get_buffer();
//	Glib::RefPtr<const SourceBuffer> get_buffer() const;
	
};

}

namespace Glib
{
	Gtk::SourceView* wrap(GtkSourceView* object, bool take_copy = false);
}
