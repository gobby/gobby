#include <vector>
#include <memory>
#include <stdexcept>

#include <glibmm/ustring.h>

class regex {
public:
	class compile_options {
	public:
		static const compile_options NONE;
		static const compile_options EXTENDED;
		static const compile_options IGNORE_CASE;
		static const compile_options NO_OFFSETS;
		static const compile_options NEWLINE;

		compile_options operator|(compile_options other) const {
			return compile_options(m_value | other.m_value);
		}
		compile_options operator&(compile_options other) const {
			return compile_options(m_value & other.m_value);
		}
		compile_options operator^(compile_options other) const {
			return compile_options(m_value ^ other.m_value);
		}
		compile_options& operator|=(compile_options other) {
			m_value |= other.m_value; return *this;
		}
		compile_options& operator&=(compile_options other) {
			m_value &= other.m_value; return *this;
		}
		compile_options& operator^=(compile_options other) {
			m_value ^= other.m_value; return *this;
		}
		compile_options operator~() const {
			return compile_options(~m_value);
		}

		operator bool() const {
			return m_value != NONE.m_value;
		}
		bool operator!() const {
			return m_value == NONE.m_value;
		}
		bool operator==(compile_options other) const {
			return m_value == other.m_value;
		}
		bool operator!=(compile_options other) const {
			return m_value != other.m_value;
		}

		unsigned int get_value() const {
			return m_value;
		}

	protected:

		explicit compile_options(unsigned int value) : m_value(value) { }

		unsigned int m_value;
	};

	class match_options {
	public:
		static const match_options NONE;
		static const match_options NOT_BOL;
		static const match_options NOT_EOL;

		match_options operator|(match_options other) const {
			return match_options(m_value | other.m_value);
		}
		match_options operator&(match_options other) const {
			return match_options(m_value & other.m_value);
		}
		match_options operator^(match_options other) const {
			return match_options(m_value ^ other.m_value);
		}
		match_options& operator|=(match_options other) {
			m_value |= other.m_value; return *this;
		}
		match_options& operator&=(match_options other) {
			m_value &= other.m_value; return *this;
		}
		match_options& operator^=(match_options other) {
			m_value ^= other.m_value; return *this;
		}
		match_options operator~() const {
			return match_options(~m_value);
		}

		operator bool() const {
			return m_value != NONE.m_value;
		}
		bool operator!() const {
			return m_value == NONE.m_value;
		}
		bool operator==(match_options other) const {
			return m_value == other.m_value;
		}
		bool operator!=(match_options other) const {
			return m_value != other.m_value;
		}

		unsigned int get_value() const {
			return m_value;
		}

protected:
		explicit match_options(unsigned int value) : m_value(value) { }

		unsigned int m_value;
};

	class compile_error : public std::runtime_error {
	public:
		/* It would be fun if these were actually useful,
		 * but I really do not know what to do with them.
		static const compile_error BAD_BACKREF;
		static const compile_error BAD_PATTERN;
		static const compile_error BAD_REPEAT;
		static const compile_error UNMATCHED_BRACE;
		static const compile_error UNMATCHED_BRACKET;
		static const compile_error INVALID_COLLATION;
		static const compile_error BAD_CHARCLASS;
		static const compile_error NON_SPECIFIC;
		static const compile_error TRAILING_BACKSLASH;
		static const compile_error UNMATCHED_PAREN;
		static const compile_error INVALID_RANGE;
		static const compile_error BUFFER_TOO_BIG;
		static const compile_error OUT_OF_MEMORY;
		static const compile_error INVALID_SUBEXP_REF;
		*/

	protected:
		explicit compile_error(void* regex, int value);
		~compile_error() throw();
		const char* make_message(void* regex, int value);

		Glib::ustring m_message;
		unsigned int m_value;

		friend class regex;
	};

	typedef std::vector<std::pair<size_t, size_t> > match_positions;

	regex(const char* regex_string,
	      compile_options cflags = compile_options::NONE);
	regex(const regex& other);
	~regex();

	void reset(const char* regex_string,
	      compile_options cflags = compile_options::NONE);

	bool match(const char* string, match_options eflags = match_options::NONE);

	bool find(const char* string, match_positions matches,
	          match_options eflags = match_options::NONE);
	bool find(const char* string, std::pair<size_t, size_t>& matchpos,
	          match_options eflags = match_options::NONE);
private:
	void* m_regex;
};
