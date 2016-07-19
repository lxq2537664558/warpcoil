#pragma once

#include <boost/lexical_cast.hpp>
#include <silicium/sink/append.hpp>
#include <silicium/sink/ptr_sink.hpp>
#include <silicium/sink/iterator_sink.hpp>
#include <warpcoil/types.hpp>

namespace warpcoil
{
    namespace cpp
    {
        template <class CharSink>
        struct comma_separator
        {
            explicit comma_separator(CharSink out)
                : m_out(out)
                , m_first(true)
            {
            }

            void add_element()
            {
                if (m_first)
                {
                    m_first = false;
                    return;
                }
                Si::append(m_out, ", ");
            }

        private:
            CharSink m_out;
            bool m_first;
        };

        template <class CharSink>
        auto make_comma_separator(CharSink out)
        {
            return comma_separator<CharSink>(out);
        }

        inline Si::memory_range find_suitable_uint_cpp_type(types::integer range)
        {
            if (range.maximum <= 0xffu)
            {
                return Si::make_c_str_range("std::uint8_t");
            }
            else if (range.maximum <= 0xffffu)
            {
                return Si::make_c_str_range("std::uint16_t");
            }
            else if (range.maximum <= 0xffffffffu)
            {
                return Si::make_c_str_range("std::uint32_t");
            }
            else
            {
                return Si::make_c_str_range("std::uint64_t");
            }
        }

        enum class type_emptiness
        {
            empty,
            non_empty
        };

        template <class CharSink>
        type_emptiness generate_type(CharSink &&code, types::type const &root)
        {
            return Si::visit<type_emptiness>(
                root,
                [&code](types::integer range)
                {
                    Si::append(code, find_suitable_uint_cpp_type(range));
                    return type_emptiness::non_empty;
                },
                [&code](std::unique_ptr<types::variant> const &root)
                {
                    Si::append(code, "Si::variant<");
                    auto comma = make_comma_separator(Si::ref_sink(code));
                    for (types::type const &element : root->elements)
                    {
                        comma.add_element();
                        generate_type(code, element);
                    }
                    Si::append(code, ">");
                    return root->elements.empty() ? type_emptiness::empty : type_emptiness::non_empty;
                },
                [&code](std::unique_ptr<types::tuple> const &root)
                {
                    Si::append(code, "std::tuple<");
                    auto comma = make_comma_separator(Si::ref_sink(code));
                    for (types::type const &element : root->elements)
                    {
                        comma.add_element();
                        generate_type(code, element);
                    }
                    Si::append(code, ">");
                    return root->elements.empty() ? type_emptiness::empty : type_emptiness::non_empty;
                },
                [&code](std::unique_ptr<types::vector> const &root)
                {
                    Si::append(code, "std::vector<");
                    generate_type(code, root->element);
                    Si::append(code, ">");
                    return type_emptiness::non_empty;
                },
                [&code](types::utf8)
                {
                    Si::append(code, "std::string");
                    return type_emptiness::non_empty;
                });
        }

        template <class CharSink>
        type_emptiness generate_parameters(CharSink &&code, std::vector<types::parameter> const &parameters)
        {
            type_emptiness total_emptiness = type_emptiness::empty;
            for (types::parameter const &param : parameters)
            {
                switch (generate_type(code, param.type_))
                {
                case type_emptiness::empty:
                    break;

                case type_emptiness::non_empty:
                    total_emptiness = type_emptiness::non_empty;
                    break;
                }
                Si::append(code, " ");
                Si::append(code, param.name);
                Si::append(code, ", ");
            }
            return total_emptiness;
        }
    }
}
