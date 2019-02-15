/*!
 * implementation of the format class
 * @copyright ryan jennings (ryan-jennings.net), 2012 under LGPL
 */

#include "format.hpp"
#include <iomanip>

namespace rj
{
    int mystoi(const std::string& s)
    {
        std::stringstream b(s);
        int out;
        b >> out;
        return out;
    }

    format::format(const string &str) : value_(str), specifiers_(), currentSpecifier_(specifiers_.begin())
    {
        initialize();
    }

    format::~format()
    {
        // specifiers_.clear();
    }

    format::format(const format &other) : value_(other.value_), specifiers_(), currentSpecifier_(specifiers_.begin())
    {
        // copy specifiers
        for (auto s : other.specifiers_) {
            specifiers_.push_back(s);
        }

        // set current position to begining
        currentSpecifier_ = specifiers_.begin();

        // advance current position according to the other position
        advance(currentSpecifier_, distance(other.specifiers_.begin(), SpecifierList::const_iterator(other.currentSpecifier_)));
    }

    format::format(format &&other)
        : value_(std::move(other.value_)), specifiers_(std::move(other.specifiers_)), currentSpecifier_(std::move(other.currentSpecifier_))
    {
        other.specifiers_.clear();
        other.currentSpecifier_ = other.specifiers_.begin();
    }

    format &format::operator=(const format &rhs)
    {
        value_ = rhs.value_;  // copy the format

        specifiers_.clear();  // clear any specifiers already set

        // copy other specifiers
        for (auto s : rhs.specifiers_) {
            specifiers_.push_back(s);
        }

        // set current position to begining
        currentSpecifier_ = specifiers_.begin();
        // advance current position according to the other position
        advance(currentSpecifier_, distance(rhs.specifiers_.begin(), SpecifierList::const_iterator(rhs.currentSpecifier_)));

        return *this;
    }

    format &format::operator=(format &&rhs)
    {
        value_ = std::move(rhs.value_);  // copy the format

        specifiers_ = std::move(rhs.specifiers_);

        // set current position to begining
        currentSpecifier_ = std::move(rhs.currentSpecifier_);

        rhs.specifiers_.clear();
        rhs.currentSpecifier_ = rhs.specifiers_.begin();

        return *this;
    }
    /*!
     * returns the number of specifiers in the format string
     */
    size_t format::specifiers() const
    {
        // the size of the specifier list minus any specifier arguments already added
        return specifiers_.size() - distance(specifiers_.begin(), SpecifierList::const_iterator(currentSpecifier_));
    }

    /*!
     * adds a specifier to the list
     * @throws invalid_argument if specifier does not contain an index
     */
    void format::add_specifier(string::size_type start, string::size_type end)
    {
        // get the string inside the delimiters
        string temp = value_.substr(start, end - start);

        // the specifier to create
        specifier spec;
        spec.index = 0;
        spec.prev = start - 1;  // exclude start tag
        spec.next = end + 1;    // exclude end tag
        spec.width = 0;
        spec.type = '\0';

        try {
            // look for {0,10:f2}
            auto divider = temp.find(':');

            if (divider != string::npos) {
                string format = temp.substr(divider + 1);

                spec.type = format[0];

                auto comma = temp.find(',', divider);

                if (comma != string::npos) {
                    spec.format = format.substr(1, comma);

                    spec.width = mystoi(temp.substr(comma + 1));

                } else {
                    spec.format = format.substr(1, format.length() - 1);
                }

                temp = temp.substr(0, divider);
            } else {
                // look for {0,-10}
                divider = temp.find(',');

                if (divider != string::npos) {
                    spec.width = mystoi(temp.substr(divider + 1));

                    temp = temp.substr(0, divider);
                }
            }


            spec.index = mystoi(temp);

        } catch (...) {
            throw invalid_argument("invalid specifier format");
        }

        specifiers_.push_back(spec);
    }


    void format::initialize()
    {
        auto len = value_.length();

        // find each open tag
        for (size_t pos = 0; pos < len; pos++) {
            if (value_[pos] != s_open_tag) {
                continue;
            }

            if (++pos >= len) break;

            // check if its an escape tag, ie  {{
            if (pos < len && value_[pos] == s_open_tag) {
                continue;
            }

            // get the closing tag
            auto end = value_.find(s_close_tag, pos);

            if (end == string::npos) {
                throw invalid_argument("no specifier closing tag");
            }

            // add the specifier
            add_specifier(pos, end);
        }

        // short circuit if no specifiers found
        if (specifiers_.size() == 0) {
            return;
        }

        // sort specifiers based on index
        specifiers_.sort([&](const specifier &first, const specifier &second) { return first.index < second.index; });

        // set the current position (note this is *after* sorting)
        currentSpecifier_ = specifiers_.begin();

        size_t index = 0;

        // check if specifier indexes follow an incremental order
        for (auto spec : specifiers_) {
            if (spec.index != index++) {
                throw invalid_argument("specifier index not ordered");
            }
        }
    }

    void format::begin_manip(ostream &out, const specifier &arg) const
    {
        if (arg.width != 0) {
            out << setw(abs(arg.width));
            if (arg.width < 0) {
                out << left;
            }
        }

        // short circuit if specifier has no format string
        if (arg.type == '\0') {
            return;
        }

        switch (arg.type) {
            // printf styles
            // TODO: define formats
            case 'E':
                out << uppercase;
            case 'e':
                if (!arg.format.empty()) {
                    try {
                        int p = mystoi(arg.format);
                        out << setprecision(p);
                    } catch (...) {
                        throw invalid_argument("invalid precision format for argument");
                    }
                } else {
                    out << setprecision(9);
                }
                out << scientific;
                break;
            case 'F':
            case 'f':
                if (!arg.format.empty()) {
                    try {
                        int p = mystoi(arg.format);
                        out << setprecision(p);
                    } catch (...) {
                        throw invalid_argument("invalid precision format for argument");
                    }
                } else {
                    out << setprecision(9);
                }
                out << fixed;
                break;

            case 'X':
                out << uppercase;
            case 'x':
                out << hex << setfill('0');
                if (arg.width == 0) {
                    out << setw(2);
                }
                break;
            case 'O':
            case 'o':
                out << oct;
                break;
        }
    }

    void format::end_manip(ostream &out, const specifier &arg)
    {
        // cleanup any stream manipulation

        switch (arg.type) {
            case 'x':
            case 'X':
                out << dec;
                break;
            case 'n':
                out << endl;
                break;
        }
    }

    void format::reset()
    {
        specifiers_.clear();
        initialize();
    }

    void format::reset(const string &value)
    {
        value_ = value;
        reset();
    }

    string format::str()
    {
        ostringstream buf;

        print(buf);

        return buf.str();
    }

    void format::unescape(ostream &buf, string::size_type start, string::size_type end)
    {
        for (auto i = start; i < end; ++i) {
            char tag = value_[i];

            if (tag != s_open_tag && tag != s_close_tag) {
                buf << tag;
                continue;
            }

            if (i + 1 < end && value_[i + 1] == tag) {
                i++;
            }

            buf << tag;
        }
    }

    void format::print(ostream &buf)
    {
        // short circuit if no specifiers
        if (specifiers_.size() == 0) {
            unescape(buf, 0, value_.length());
            return;
        }
        // sort based on position
        specifiers_.sort([&](const specifier &first, const specifier &second) { return first.prev < second.prev; });

        size_t last = 0;

        // loop through all added arguments
        for (auto spec = specifiers_.begin(); spec != currentSpecifier_; ++spec) {
            if (spec->prev != 0) {
                // add filler between specifiers
                unescape(buf, last, spec->prev);
            }

            // append replacement
            buf << spec->replacement;

            last = spec->next;
        }

        // add ending
        if (last < value_.length()) {
            unescape(buf, last, value_.length());
        }
    }

    format::operator string()
    {
        return str();
    }


    ostream &operator<<(ostream &out, format &f)
    {
        f.print(out);
        return out;
    }
}