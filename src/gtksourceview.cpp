
#include "sourceview.hpp"

const Glib::Class& SourceView_Class::init()
{
	if(!gtype_)
	{
		class_init_func_ = &SourceView_Class::class_init_function;
		register_derived_type(gtk_source_view_get_type() );
	}

	return *this;
}

void Gtk::SourceView_Class::class_init_function(void* g_class, void* class_data)
{
	BaseClassType* const klass = static_cast<BaseClassType*>(g_class);
	CppClassParent::class_init_function(klass, class_data);

	// Hier irgendwelche Signale.
}

Glib::ObjectBase* SourceView_Class::wrap_new(GObject* o)
{
	return manage(new SourceView( (GtkSourceView*)(o)));
}

Gtk::SourceView::SourceView()
 : Glib::ObjectBase(0),
   Gtk::Container(Glib::ConstructParams(sourceview_class_.init()))
{
}

Gtk::SourceView::SourceView(const Glib::ConstructParams& construct_params)
 : Gtk::TextView(construct_params)
{
}

/*Gtk::SourceView::SourceView(const Glib::RefPtr<SourceBuffer>& buffer)
   Gtk::TextView(Glib::ConstructParams(sourceview_class_.init(), (char*)0))
{
	set_buffer(buffer);
}*/

Gtk::SourceView::~SourceView()
{
	// TODO: TextView-dtor calls destroy_ as well...
	destroy_();
}

GType SourceView::get_type()
{
	return sourceview_class_.init().get_type();
}

GType SourceView::get_base_type()
{
	return gtk_source_view_get_type();
}

/*Glib::RefPtr<Gtk::SourceBuffer> Gtk::SourceView::get_buffer()
{
	return Glib::RefPtr<Gtk::SourceBuffer>::cast_static(Gtk::TextView::get_buffer() );
}

Glib::RefPtr<const Gtk::SourceBuffer> Gtk::SourceView::get_buffer() const
{
	return Glib::RefPtr<Gtk::SourceBuffer>::cast_static(Gtk::TextView::get_buffer() );
}
*/
