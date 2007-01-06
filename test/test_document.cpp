#include "gselector.hpp"
#include "document.hpp"
#include <obby/host_buffer.hpp>
#include <gtkmm/main.h>

#define ARRAY_SIZE(array) sizeof(array) / sizeof(array[0])

// This test depends on working obby::text!
// Verify this with obby/test/text before running these tests!

namespace
{
	typedef obby::basic_host_buffer<Gobby::Document, Gobby::GSelector>
		BaseBuffer;

	class TestBuffer: public BaseBuffer
	{
	public:
		TestBuffer();

		virtual void open(unsigned int port);
	};

	TestBuffer::TestBuffer():
		BaseBuffer("User1", obby::colour(255, 0, 0))
	{
	}

	void TestBuffer::open(unsigned int port)
	{
		BaseBuffer::open(port);

		m_user_table.add_user(
			m_user_table.find_free_id(),
			"User2",
			obby::colour(0, 255, 0)
		);

		m_user_table.add_user(
			m_user_table.find_free_id(),
			"User3",
			obby::colour(0, 0, 255)
		);

		m_user_table.add_user(
			m_user_table.find_free_id(),
			"User4",
			obby::colour(0, 127, 0)
		);
	}

	class DescError: public std::logic_error
	{
	public:
		DescError(const std::string& error_message):
			std::logic_error(error_message) {}
	};

	obby::text make_text_from_desc(const std::string& desc,
	                               const obby::user_table& users)
	{
		obby::text my_text;

		if(desc.empty() )
			return my_text;

		std::string::size_type pos = 0;
		const obby::user* cur_user = NULL;

		std::string::size_type prev = 0;
		while( (pos = desc.find('[', pos)) != std::string::npos)
		{
			if(pos > prev)
			{
				if(cur_user == NULL)
					throw DescError("Unusered chunk");

				my_text.append(
					desc.substr(prev, pos - prev),
					cur_user
				);
			}

			obby::text::size_type close = desc.find(']', pos);
			if(close == std::string::npos)
				throw std::logic_error("Missing ']'");

			std::string name = "User" + desc.substr(
				pos + 1,
				close - pos - 1
			);

			cur_user = users.find(
				name,
				obby::user::flags::NONE,
				obby::user::flags::NONE
			);

			pos = prev = close + 1;
		}

		if(cur_user == NULL)
			throw DescError("Unusered chunk");

		my_text.append(
			desc.substr(prev),
			cur_user
		);

		return my_text;
	}

	std::string make_desc_from_text(const obby::text& txt)
	{
		std::string str;

		for(obby::text::chunk_iterator iter = txt.chunk_begin();
		    iter != txt.chunk_end();
		    ++ iter)
		{
			str += "[";
			str += iter->get_author()->get_name()[4];
			str += "]";
			str += iter->get_text();
		}

		return str;
	}

	// TODO: Have these tests somewhere were both obby and Gobby can
	// access them
	struct InsertTest {
		static const char* NAME;

		const char* first;
		const obby::text::size_type pos;
		const char* second;
		const char* expected;

		void perform(Gobby::Document& doc,
		             const obby::user_table& table) const
		{
			doc.insert(0, make_text_from_desc(first, table));
			doc.insert(pos, make_text_from_desc(second, table));
		}
	};

	struct SubstrTest {
		static const char* NAME;

		const char* str;
		const obby::text::size_type pos;
		const obby::text::size_type len;
		const char* expected;

		void perform(Gobby::Document& doc,
		             const obby::user_table& table) const
		{
			doc.insert(0, make_text_from_desc(str, table));
			obby::text text = doc.get_slice(pos, len);

			doc.clear();
			doc.insert(0, text);
		}
	};

	struct EraseTest {
		static const char* NAME;

		const char* str;
		const obby::text::size_type pos;
		const obby::text::size_type len;
		const char* expected;

		void perform(Gobby::Document& doc,
		             const obby::user_table& table) const
		{
			doc.insert(0, make_text_from_desc(str, table));
			doc.erase(pos, len);
		}
	};

	struct AppendTest {
		static const char* NAME;

		const char* str;
		const char* app;
		const char* expected;

		void perform(Gobby::Document& doc,
		             const obby::user_table& table) const
		{
			doc.insert(0, make_text_from_desc(str, table));
			doc.append(make_text_from_desc(app, table));
		}
	};

#if 0
	struct PrependTest {
		static const char* NAME;

		const char* str;
		const char* pre;
		const char* expected;

		void perform(Gobby::Document& doc,
		             const obby::user_table& table) const
		{
			doc.insert(0, make_text_from_desc(str, table));
			doc.prepend(make_text_from_desc(pre, table));
		}
	};
#endif

	const char* InsertTest::NAME = "insert";
	const char* SubstrTest::NAME = "substr";
	const char* EraseTest::NAME = "erase";
	const char* AppendTest::NAME = "append";
#if 0
	const char* PrependTest::NAME = "prepend";
#endif

	InsertTest INSERT_TESTS[] = {
		{ "", 0, "[1]bar", "[1]bar" },
		{ "", 1, "[1]bar", NULL },
		{ "[1]foo", 3, "[1]bar", "[1]foobar" },
		{ "[1]foo", 0, "[1]bar", "[1]barfoo" },
		{ "[1]for", 2, "[1]oba", "[1]foobar" },
		{ "[1]foo", 0, "[2]bar", "[2]bar[1]foo" },
		{ "[1]foo", 4, "[1]bar", NULL },
		{ "[1]gnah", 3, "[2]gnah", "[1]gna[2]gnah[1]h" },
		{ "[1]gnah", 4, "[2]gnah", "[1]gnah[2]gnah" },
		{ "[1]b[2]a", 1, "[1]foo", "[1]bfoo[2]a" },
		{ "[1]b[2]a", 1, "[2]foo", "[1]b[2]fooa" },
		{ "[1]b[2]a", 1, "[1]f[2]oo", "[1]bf[2]ooa" },
		{ "[1]b[2]a", 1, "[2]f[1]oo", "[1]b[2]f[1]oo[2]a" },
		{ "[1]b[2]a", 1, "[1]f[2]o[1]o", "[1]bf[2]o[1]o[2]a" },
		{ "[1]b[2]a", 1, "[2]f[1]o[2]o", "[1]b[2]f[1]o[2]oa" },
		{ "[1]Die Frage[2] ist halt,[3] [2]ob[1] das so", 11, "[3]n", "[1]Die Frage[2] i[3]n[2]st halt,[3] [2]ob[1] das so" }
	};

	SubstrTest SUBSTR_TESTS[] = {
		{ "", 0, 0, "" },
		{ "[1]bar", 0, 3, "[1]bar" },
		{ "[1]bar", 0, 1, "[1]b" },
		{ "[1]bar", 0, 0, "" },
		{ "[1]b[2]a[1]r", 0, 3, "[1]b[2]a[1]r" },
		{ "[1]b[2]a[1]r", 0, 2, "[1]b[2]a" },
		{ "[1]b[2]a[1]r", 1, 1, "[2]a" },
		{ "[1]foo[2]bar", 0, 3, "[1]foo" },
		{ "[1]foo[2]bar", 3, 3, "[2]bar" },
		{ "[1]foo[2]bar", 1, 3, "[1]oo[2]b" },
		{ "[1]foo[2]bar", 2, 3, "[1]o[2]ba" },
		{ "[1]foo[2]bar", 0, 4, "[1]foo[2]b" },
		{ "[1]foo[2]bar", 1, 4, "[1]oo[2]ba" },
		{ "[1]foo[2]bar", 2, 4, "[1]o[2]bar" },
		{ "[1]foo[2]bar", 1, 2, "[1]oo" },
		{ "[1]foo[2]bar", 2, 2, "[1]o[2]b" },
		{ "[1]foo[2]bar", 3, 2, "[2]ba" },
		{ "[1]foo[2]bar", 5, 2, NULL },
		{ "[1]foo[2]bar", 2, obby::text::npos, "[1]o[2]bar" },
		{ "[1]foo[2]bar", 3, obby::text::npos, "[2]bar" }
	};

	EraseTest ERASE_TESTS[] = {
		{ "", 0, 0, "" },
		{ "", 1, 0, NULL },
		{ "", 0, 1, NULL },
		{ "[1]foo", 0, 1, "[1]oo" },
		{ "[1]foo", 1, 1, "[1]fo" },
		{ "[1]foo", 1, 2, "[1]f" },
		{ "[1]foo", 0, 3, "" },
		{ "[1]foo[2]bar", 0, 1, "[1]oo[2]bar" },
		{ "[1]foo[2]bar", 1, 1, "[1]fo[2]bar" },
		{ "[1]foo[2]bar", 2, 1, "[1]fo[2]bar" },
		{ "[1]foo[2]bar", 3, 1, "[1]foo[2]ar" },
		{ "[1]foo[2]bar", 4, 1, "[1]foo[2]br" },
		{ "[1]foo[2]bar", 5, 1, "[1]foo[2]ba" },
		{ "[1]foo[2]bar", 0, 3, "[2]bar" },
		{ "[1]foo[2]bar", 1, 3, "[1]f[2]ar" },
		{ "[1]foo[2]bar", 2, 3, "[1]fo[2]r" },
		{ "[1]foo[2]bar", 3, 3, "[1]foo" },
		{ "[1]foo[2]bar[1]baz", 2, 3, "[1]fo[2]r[1]baz" },
		{ "[1]foo[2]bar[1]baz", 3, 3, "[1]foobaz" },
		{ "[1]foo[2]bar[1]baz", 4, 3, "[1]foo[2]b[1]az" },
		{ "[1]foo[2]b[3]az[1]o", 2, 5, "[1]fo" },
		{ "[1]foo[2]bar[3]baz[2]qux[3]fo[2]gneh[1]grah", 0, 10, "[2]ux[3]fo[2]gneh[1]grah" },
		{ "[1]foo[2]bar", 2, obby::text::npos, "[1]fo" }
	};

       AppendTest APPEND_TESTS[] = {
		{ "", "", "" },
		{ "", "[1]bar", "[1]bar" },
		{ "", "[1]bar[2]foo", "[1]bar[2]foo" },
		{ "[1]foo[2]bar", "[1]bar[2]foo", "[1]foo[2]bar[1]bar[2]foo" },
		{ "[1]foo[2]bar", "[2]bar[1]foo", "[1]foo[2]barbar[1]foo" }
	};

#if 0
	PrependTest PREPEND_TESTS[] = {
		{ "", "", "" },
		{ "", "[1]bar", "[1]bar" },
		{ "", "[1]bar[2]foo", "[1]bar[2]foo" },
		{ "[1]foo[2]bar", "[1]bar[2]foo", "[1]bar[2]foo[1]foo[2]bar" },
		{ "[1]foo[2]bar", "[2]bar[1]foo", "[2]bar[1]foofoo[2]bar" }
	};
#endif

	template<typename Test>
	void test_generic(const Test& test,
	                  TestBuffer& buffer)
	{
		try
		{
			const obby::user_table& table =
				buffer.get_user_table();

			Gobby::Document result(buffer);
			test.perform(result, table);

			if(test.expected == NULL)
			{
				throw std::logic_error(
					std::string(Test::NAME) + " should "
					"fail, bit it has not"
				);
			}

			Gobby::Document expected(buffer);

			expected.insert(
				0,
				make_text_from_desc(test.expected, table)
			);

			obby::text result_text =
				result.get_slice(0, result.size());
			obby::text expected_text =
				expected.get_slice(0, expected.size());

			if(result_text != expected_text)
			{
				throw std::logic_error(
					"Result does not match expectation:\n"
					"Expected " +
					make_desc_from_text(expected_text) +
					", got " +
					make_desc_from_text(result_text)
				);
			}
		}
		catch(DescError& e)
		{
			throw e;
		}
		catch(std::logic_error& e)
		{
			if(test.expected != NULL)
				throw e;
		}
	}

	template<typename Test>
	bool test_suite(const Test* tests,
	                std::size_t num,
	                TestBuffer& buffer)
	{
		bool result = true;
		for(std::size_t i = 0; i < num; ++ i)
		{
			try
			{
				test_generic(tests[i], buffer);
			}
			catch(std::exception& e)
			{
				std::cerr << Test::NAME << " test #" << (i + 1)
				          << " failed:\n" << e.what()
					  << "\n" << std::endl;
			
				result = false;
				continue;
			}

			std::cerr << Test::NAME << " test #" << (i + 1)
			          << " passed" << std::endl;
		}

		return result;
	}
}

int main(int argc, char* argv[])
{
	Gtk::Main kit(argc, argv);

	TestBuffer test;
	test.open(6522); // I wish I could get rid of this call :( - armin

	bool result = true;

	result = test_suite(INSERT_TESTS, ARRAY_SIZE(INSERT_TESTS), test) &&
		result;
	result = test_suite(SUBSTR_TESTS, ARRAY_SIZE(SUBSTR_TESTS), test) &&
		result;
	result = test_suite(ERASE_TESTS, ARRAY_SIZE(ERASE_TESTS), test) &&
		result;
	result = test_suite(APPEND_TESTS, ARRAY_SIZE(APPEND_TESTS), test) &&
		result;

	return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
