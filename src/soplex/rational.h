
/* ----------------------------------------------------------------------
 * This file is autogenerated from the file multiprecision.hpp.in during
 * the cmake configuration of your project. If you need to make changes
 * edit the original file.
 * ----------------------------------------------------------------------
 */
#ifndef __SOPLEX_MULTIPRECISION_HPP_
#define __SOPLEX_MULTIPRECISION_HPP_
#define SOPLEX_DEBUG
#include <numeric>
#include <vector>
#include <string>
#include "soplex/spxdefines.h"

#ifdef SOPLEX_WITH_GMP
#include <gmp.h>
#endif

#ifdef SOPLEX_WITH_BOOST
#include <boost/multiprecision/number.hpp>

#ifdef SOPLEX_WITH_GMP
#include <boost/multiprecision/gmp.hpp>

namespace soplex
{

using namespace boost::multiprecision;
using Rational = number<gmp_rational, et_off>;
using Integer = number<gmp_int, et_off>;
inline void SpxLcm(Integer& result, Integer a, Integer b)
{
   mpz_lcm(result.backend().data(), a.backend().data(), b.backend().data());
}
inline void SpxGcd(Integer& result, Integer a, Integer b)
{
   mpz_gcd(result.backend().data(), a.backend().data(), b.backend().data());
}

} // namespace soplex
#else
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/detail/default_ops.hpp>

namespace soplex
{

using namespace boost::multiprecision;
using Rational = cpp_rational;
using Integer = cpp_int;
inline void SpxLcm(Integer& result, Integer a, Integer b)
{
   result = boost::multiprecision::lcm(a, b);
}
inline void SpxGcd(Integer& result, Integer a, Integer b)
{
   result = boost::multiprecision::gcd(a, b);
}

} // namespace soplex
#endif

namespace soplex
{

inline void printRational(Rational r)
{
   std::cout << r << std::endl;
}

inline void printInteger(Integer r)
{
   std::cout << r << std::endl;
}
inline bool isAdjacentTo(const Rational& r, const double& d)
{
   double x = (double) r;
   double a;
   double b;
   Rational tmp = x;

   // the rational value is representable in double precision
   if(tmp == r)
      return true;
   // the rounded value is smaller than the rational value
   else if(tmp < r)
   {
      a = x;
      b = (double)nextafter(a, 1e100);
   }
   // the rounded value is larger than the rational value
   else
   {
      b = x;
      a = (double)nextafter(b, -1e100);
   }

   return ((a == d) || (b == d));
}

inline void invert(Rational& r)
{
   r = Rational(denominator(r), numerator(r));
}

/// round up to next power of two
inline void powRound(Rational& r)
{
   Integer roundval;
   Integer den;
   Integer num;

   MSG_DEBUG(std::cout << "rounding " << str(r) <<
             " to power of two" << "\n");

   num = numerator(r);
   den = denominator(r);
   roundval = num / den;

   MSG_DEBUG(std::cout << "   --> " << str(roundval) << "\n");

   size_t binlog = roundval == 0 ? 1 : msb(roundval) + 1;
   Integer base = 2;

   MSG_DEBUG(std::cout << "   --> 2^" << binlog << "\n");

   roundval = boost::multiprecision::pow(base, (unsigned int)binlog);

   MSG_DEBUG(std::cout << "   --> " << str(roundval) << "\n");

   r = roundval;

   MSG_DEBUG(std::cout << "   --> " << str(r) << "\n");
}

/* find substring, ignore case */
static
std::string::const_iterator findSubStringIC(const std::string& substr, const std::string& str)
{
   auto it = std::search(
                str.begin(), str.end(),
                substr.begin(),   substr.end(),
                [](char ch1, char ch2)
   {
      return std::toupper(ch1) == std::toupper(ch2);
   }
             );
   return it;
}

inline Rational ratFromString(const char* desc)
{
   Rational res;

   if(0 == strcmp(desc, "inf"))
   {
      res = 1e100;
   }
   else if(0 == strcmp(desc, "-inf"))
   {
      res = -1e100;
   }
   else
   {
      std::string s(desc);

      /* case 1: string is given in nom/den format */
      if(s.find('.') == std::string::npos)
         res = Rational(desc);
      /* case 2: string is given as base-10 decimal number */
      else
      {
         std::string::const_iterator it = findSubStringIC("e", s);
         int mult = 0;

         if(it != s.end())
         {
            int exponentidx = int(it - s.begin());
            mult = std::stoi(s.substr(exponentidx + 1, s.length()));
            s = s.substr(0, exponentidx);
         }

         // std::cout << s << std::endl;
         if(s[0] == '.')
            s.insert(0, "0");

         size_t pos = s.find('.');
         size_t exp = s.length() - 1 - pos;
         std::string den("1");

         for(size_t i = 0; i < exp; ++i)
            den.append("0");

         s.erase(pos, 1);
         assert(std::all_of(s.begin() + 1, s.end(), ::isdigit));

         // remove padding 0s
         if(s[0] == '-')
            s.erase(1, std::min(s.substr(1).find_first_not_of('0'), s.size() - 1));
         else
            s.erase(0, std::min(s.find_first_not_of('0'), s.size() - 1));

         s.append("/");
         s.append(den);
         res = Rational(s);
         res *= pow(10, mult);
      }
   }

   return res;
}

} // namespace soplex
#else

#ifndef SOPLEX_WITH_GMP
using mpq_t = char;
#endif

using Integer = int;
// this is a placeholder class to ensure compilation when boost ist not linked. Rationals need BOOST in order to function.
class Rational
{

private:
   double val;

public:

   ///@name Construction and destruction
   ///@{

   inline void rationalErrorMessage() const
   {
      MSG_ERROR(std::cerr << "Using rational methods without linking boost is not supported" << std::endl;
               )
   };

   /// default constructor
   inline Rational()
   {
      rationalErrorMessage();
   };
   /// copy constructor
   inline Rational(const Rational& r)
   {
      rationalErrorMessage();
   };
   /// constructor from long double
   inline Rational(const long double& r)
   {
      rationalErrorMessage();
   };
   /// constructor from double
   inline Rational(const double& r)
   {
      rationalErrorMessage();
   };
   ///constructor from int
   inline Rational(const int& i)
   {
      rationalErrorMessage();
   };
   /// constructor from Integer
   inline Rational(const Integer& num, const Integer& den)
   {
      rationalErrorMessage();
   };
   /// constructor from mpq_t (GMP only)
   inline Rational(const mpq_t& q)
   {
      rationalErrorMessage();
   };
#ifdef SOPLEX_WITH_BOOST
   // constructor from boost number
   inline template <typename T, boost::multiprecision::expression_template_option eto>
   Rational(const boost::multiprecision::number<T, eto>& q)
   {
      rationalErrorMessage();
   };
#endif
   /// destructor
   inline ~Rational()
   {
      rationalErrorMessage();
   };

   /// assignment operator
   inline Rational& operator=(const Rational&)
   {
      rationalErrorMessage();
      return *this;
   };
   /// assignment operator from long double
   inline Rational& operator=(const long double& r)
   {
      rationalErrorMessage();
      return *this;
   };
   /// assignment operator from double
   inline Rational& operator=(const double& r)
   {
      rationalErrorMessage();
      return *this;
   };
   /// assignment operator from int
   inline Rational& operator=(const int& i)
   {
      rationalErrorMessage();
      return *this;
   };
   /// assignment operator from mpq_t
   inline Rational& operator=(const mpq_t& q)
   {
      rationalErrorMessage();
      return *this;
   };

   inline void assign(const Rational&)
   {
      rationalErrorMessage();
   };
   inline void assign(const long double& r)
   {
      rationalErrorMessage();
   };
   inline void assign(const double& r)
   {
      rationalErrorMessage();
   };
   inline void assign(const int& i)
   {
      rationalErrorMessage();
   };

   ///@name Typecasts
   ///@{

   inline operator double() const
   {
      return 0;
   };
   inline operator long double() const
   {
      return 0;
   };
   inline operator float() const
   {
      return 0;
   };
#ifdef SOPLEX_WITH_BOOST
#ifndef SOPLEX_WITH_CPPMPF
   // Operator to typecast Rational to one of the Boost Number types
   inline template <typename T, boost::multiprecision::expression_template_option eto>
   operator boost::multiprecision::number<T, eto>() const
   {
      rationalErrorMessage();
      return val;
   };
#else
   // Operator to typecast Rational to one of the Boost Number types
   inline template <unsigned bits, boost::multiprecision::expression_template_option eto>
   operator boost::multiprecision::number<boost::multiprecision::backends::cpp_dec_float<bits>, eto>()
   const
   {
      rationalErrorMessage();
      return val;
   };
#endif
#endif

   ///@name Typecasts
   ///@{

   ///@}


   ///@name Arithmetic operators
   ///@{

   /// addition operator
   inline Rational operator+(const Rational& r) const
   {
      rationalErrorMessage();
      return *this;
   }
   /// addition assignment operator
   inline Rational operator+=(const Rational& r)
   {
      rationalErrorMessage();
      return *this;
   }
   /// addition operator for doubles
   inline Rational operator+(const double& r) const
   {
      rationalErrorMessage();
      return *this;
   }
   /// addition assignment operator  for doubles
   inline Rational operator+=(const double& r)
   {
      rationalErrorMessage();
      return *this;
   }
   /// addition operator for ints
   inline Rational operator+(const int& r) const
   {
      rationalErrorMessage();
      return *this;
   }
   /// addition assignment operator  for ints
   inline Rational operator+=(const int& r)
   {
      rationalErrorMessage();
      return *this;
   }
   /// subtraction operator
   inline Rational operator-(const Rational& r) const
   {
      rationalErrorMessage();
      return *this;
   }
   /// subtraction assignment operator
   inline Rational operator-=(const Rational& r)
   {
      rationalErrorMessage();
      return *this;
   }
   /// subtraction operator for doubles
   inline Rational operator-(const double& r) const
   {
      rationalErrorMessage();
      return *this;
   }
   /// subtraction assignment operator for doubles
   inline Rational operator-=(const double& r)
   {
      rationalErrorMessage();
      return *this;
   }
   /// subtraction operator for ints
   inline Rational operator-(const int& r) const
   {
      rationalErrorMessage();
      return *this;
   }
   /// subtraction assignment operator for ints
   inline Rational operator-=(const int& r)
   {
      rationalErrorMessage();
      return *this;
   }
   /// multiplication operator
   inline Rational operator*(const Rational& r) const
   {
      rationalErrorMessage();
      return *this;
   }
   /// multiplication assignment operator operator
   inline Rational operator*=(const Rational& r)
   {
      rationalErrorMessage();
      return *this;
   }
   /// multiplication operator for doubles
   inline Rational operator*(const double& r) const
   {
      rationalErrorMessage();
      return *this;
   }
   /// multiplication assignment operator for doubles
   inline Rational operator*=(const double& r)
   {
      rationalErrorMessage();
      return *this;
   }
   /// multiplication operator for ints
   inline Rational operator*(const int& r) const
   {
      rationalErrorMessage();
      return *this;
   }
   /// multiplication assignment operator for ints
   inline Rational operator*=(const int& r)
   {
      rationalErrorMessage();
      return *this;
   }
   /// division operator
   inline Rational operator/(const Rational& r) const
   {
      rationalErrorMessage();
      return *this;
   }
   /// division assignment operator
   inline Rational operator/=(const Rational& r)
   {
      rationalErrorMessage();
      return *this;
   }
   /// division operator for doubles
   inline Rational operator/(const double& r) const
   {
      rationalErrorMessage();
      return *this;
   }
   /// division assignment operator for doubles
   inline Rational operator/=(const double& r)
   {
      rationalErrorMessage();
      return *this;
   }
   /// division operator for ints
   inline Rational operator/(const int& r) const
   {
      rationalErrorMessage();
      return *this;
   }
   /// division assignment operator for ints
   inline Rational operator/=(const int& r)
   {
      rationalErrorMessage();
      return *this;
   }
   /// add product of two rationals
   Rational& addProduct(const Rational& r, const Rational& s)
   {
      rationalErrorMessage();
      return *this;
   }

   /// subtract product of two rationals
   Rational& subProduct(const Rational& r, const Rational& s)
   {
      rationalErrorMessage();
      return *this;
   }

   /// add quotient of two rationals, r divided by s
   Rational& addQuotient(const Rational& r, const Rational& s)
   {
      rationalErrorMessage();
      return *this;
   }

   /// subtract quotient of two rationals, r divided by s
   Rational& subQuotient(const Rational& r, const Rational& s)
   {
      rationalErrorMessage();
      return *this;
   }

   ///@}


   ///@name Methods for checking exactness of doubles
   ///@{

   /// checks if \p d is exactly equal to the Rational and if not, if it is one of the two adjacent doubles
   inline bool isAdjacentTo(const double& d) const
   {
      rationalErrorMessage();
      return false;
   };

   ///@}


   ///@name Methods for querying size
   ///@{

   /// Size in specified base (bit size for base 2)
   int sizeInBase(const int base = 2) const
   {
      rationalErrorMessage();
      return 0;
   };

   ///@}


   ///@name Conversion from and to String
   ///@{
   inline friend std::ostream& operator<<(std::ostream& os, const Rational& r)
   {
      r.rationalErrorMessage();
      return os;
   };
   inline std::string str() const
   {
      this->rationalErrorMessage();
      return std::string("");
   };
   ///@}

   ///@name Friends
   ///@{

   inline friend int compareRational(const Rational& r, const Rational& s)
   {
      r.rationalErrorMessage();
      return 0;
   };
   inline friend bool operator!=(const Rational& r, const Rational& s)
   {
      r.rationalErrorMessage();
      return false;
   };
   inline friend bool operator==(const Rational& r, const Rational& s)
   {
      r.rationalErrorMessage();
      return false;
   };
   inline friend bool operator<(const Rational& r, const Rational& s)
   {
      r.rationalErrorMessage();
      return false;
   };
   inline friend bool operator<=(const Rational& r, const Rational& s)
   {
      r.rationalErrorMessage();
      return false;
   };
   inline friend bool operator>(const Rational& r, const Rational& s)
   {
      r.rationalErrorMessage();
      return false;
   };
   inline friend bool operator>=(const Rational& r, const Rational& s)
   {
      r.rationalErrorMessage();
      return false;
   };

   inline friend bool operator!=(const Rational& r, const double& s)
   {
      r.rationalErrorMessage();
      return false;
   };
   inline friend bool operator==(const Rational& r, const double& s)
   {
      r.rationalErrorMessage();
      return false;
   };
   inline friend bool operator<(const Rational& r, const double& s)
   {
      r.rationalErrorMessage();
      return false;
   };
   inline friend bool operator<=(const Rational& r, const double& s)
   {
      r.rationalErrorMessage();
      return false;
   };
   inline friend bool operator>(const Rational& r, const double& s)
   {
      r.rationalErrorMessage();
      return false;
   };
   inline friend bool operator>=(const Rational& r, const double& s)
   {
      r.rationalErrorMessage();
      return false;
   };

   inline friend bool operator!=(const double& r, const Rational& s)
   {
      s.rationalErrorMessage();
      return false;
   };
   inline friend bool operator==(const double& r, const Rational& s)
   {
      s.rationalErrorMessage();
      return false;
   };
   inline friend bool operator<(const double& r, const Rational& s)
   {
      s.rationalErrorMessage();
      return false;
   };
   inline friend bool operator<=(const double& r, const Rational& s)
   {
      s.rationalErrorMessage();
      return false;
   };
   inline friend bool operator>(const double& r, const Rational& s)
   {
      s.rationalErrorMessage();
      return false;
   };
   inline friend bool operator>=(const double& r, const Rational& s)
   {
      s.rationalErrorMessage();
      return false;
   };

   inline friend bool operator!=(const Rational& r, const long double& s)
   {
      r.rationalErrorMessage();
      return false;
   };
   inline friend bool operator==(const Rational& r, const long double& s)
   {
      r.rationalErrorMessage();
      return false;
   };
   inline friend bool operator<(const Rational& r, const long double& s)
   {
      r.rationalErrorMessage();
      return false;
   };
   inline friend bool operator<=(const Rational& r, const long double& s)
   {
      r.rationalErrorMessage();
      return false;
   };
   inline friend bool operator>(const Rational& r, const long double& s)
   {
      r.rationalErrorMessage();
      return false;
   };
   inline friend bool operator>=(const Rational& r, const long double& s)
   {
      r.rationalErrorMessage();
      return false;
   };

   inline friend bool operator!=(const long double& r, const Rational& s)
   {
      s.rationalErrorMessage();
      return false;
   };
   inline friend bool operator==(const long double& r, const Rational& s)
   {
      s.rationalErrorMessage();
      return false;
   };
   inline friend bool operator<(const long double& r, const Rational& s)
   {
      s.rationalErrorMessage();
      return false;
   };
   inline friend bool operator<=(const long double& r, const Rational& s)
   {
      s.rationalErrorMessage();
      return false;
   };
   inline friend bool operator>(const long double& r, const Rational& s)
   {
      s.rationalErrorMessage();
      return false;
   };
   inline friend bool operator>=(const long double& r, const Rational& s)
   {
      s.rationalErrorMessage();
      return false;
   };

   inline friend bool operator!=(const Rational& r, const float& s)
   {
      r.rationalErrorMessage();
      return false;
   };
   inline friend bool operator==(const Rational& r, const float& s)
   {
      r.rationalErrorMessage();
      return false;
   };
   inline friend bool operator<(const Rational& r, const float& s)
   {
      r.rationalErrorMessage();
      return false;
   };
   inline friend bool operator<=(const Rational& r, const float& s)
   {
      r.rationalErrorMessage();
      return false;
   };
   inline friend bool operator>(const Rational& r, const float& s)
   {
      r.rationalErrorMessage();
      return false;
   };
   inline friend bool operator>=(const Rational& r, const float& s)
   {
      r.rationalErrorMessage();
      return false;
   };

   inline friend bool operator!=(const float& r, const Rational& s)
   {
      s.rationalErrorMessage();
      return false;
   };
   inline friend bool operator==(const float& r, const Rational& s)
   {
      s.rationalErrorMessage();
      return false;
   };
   inline friend bool operator<(const float& r, const Rational& s)
   {
      s.rationalErrorMessage();
      return false;
   };
   inline friend bool operator<=(const float& r, const Rational& s)
   {
      s.rationalErrorMessage();
      return false;
   };
   inline friend bool operator>(const float& r, const Rational& s)
   {
      s.rationalErrorMessage();
      return false;
   };
   inline friend bool operator>=(const float& r, const Rational& s)
   {
      s.rationalErrorMessage();
      return false;
   };

   inline friend Rational operator+(const double& d, const Rational& r)
   {
      r.rationalErrorMessage();
      return r;
   };
   inline friend Rational operator-(const double& d, const Rational& r)
   {
      r.rationalErrorMessage();
      return r;
   };
   inline friend Rational operator*(const double& d, const Rational& r)
   {
      r.rationalErrorMessage();
      return r;
   };
   inline friend Rational operator/(const double& d, const Rational& r)
   {
      r.rationalErrorMessage();
      return r;
   };

   inline friend bool operator!=(const Rational& r, const int& s)
   {
      r.rationalErrorMessage();
      return false;
   };
   inline friend bool operator==(const Rational& r, const int& s)
   {
      r.rationalErrorMessage();
      return false;
   };
   inline friend bool operator<(const Rational& r, const int& s)
   {
      r.rationalErrorMessage();
      return false;
   };
   inline friend bool operator<=(const Rational& r, const int& s)
   {
      r.rationalErrorMessage();
      return false;
   };
   inline friend bool operator>(const Rational& r, const int& s)
   {
      r.rationalErrorMessage();
      return false;
   };
   inline friend bool operator>=(const Rational& r, const int& s)
   {
      r.rationalErrorMessage();
      return false;
   };

   inline friend bool operator!=(const int& r, const Rational& s)
   {
      s.rationalErrorMessage();
      return false;
   };
   inline friend bool operator==(const int& r, const Rational& s)
   {
      s.rationalErrorMessage();
      return false;
   };
   inline friend bool operator<(const int& r, const Rational& s)
   {
      s.rationalErrorMessage();
      return false;
   };
   inline friend bool operator<=(const int& r, const Rational& s)
   {
      s.rationalErrorMessage();
      return false;
   };
   inline friend bool operator>(const int& r, const Rational& s)
   {
      s.rationalErrorMessage();
      return false;
   };
   inline friend bool operator>=(const int& r, const Rational& s)
   {
      s.rationalErrorMessage();
      return false;
   };

   inline friend Rational operator+(const int& d, const Rational& r)
   {
      r.rationalErrorMessage();
      return r;
   };
   inline friend Rational operator-(const int& d, const Rational& r)
   {
      r.rationalErrorMessage();
      return r;
   };
   inline friend Rational operator*(const int& d, const Rational& r)
   {
      r.rationalErrorMessage();
      return r;
   };
   inline friend Rational operator/(const int& d, const Rational& r)
   {
      r.rationalErrorMessage();
      return r;
   };

   inline friend Rational spxAbs(const Rational& r)
   {
      r.rationalErrorMessage();
      return r;
   };
   inline friend int sign(const Rational& r)
   {
      r.rationalErrorMessage();
      return 0;
   };
   inline friend Rational operator-(const Rational& q)
   {
      q.rationalErrorMessage();
      return q;
   };///@name Construction and destruction
   ///@{
};

inline Integer numerator(const Rational& r)
{
   r.rationalErrorMessage();
   return 0;
}
inline Integer denominator(const Rational& r)
{
   r.rationalErrorMessage();
   return 0;
}
inline Rational ratFromString(const char* desc)
{
   return Rational();
}
inline void SpxLcm(Integer& result, Integer a, Integer b) {}
inline void SpxGcd(Integer& result, Integer a, Integer b) {}
inline void divide_qr(Integer& result, Integer& result2, Integer a, Integer b) {}
inline void invert(Rational& r)
{
   r.rationalErrorMessage();
}
inline void powRound(Rational& r)
{
   r.rationalErrorMessage();
}
#endif

namespace soplex
{

/// Size in specified base (bit size for base 2)
inline int sizeInBase(const Rational R, const int base)
{
   assert(base == 2);

#ifndef SOPLEX_WITH_BOOST
   MSG_ERROR(std::cerr << "ERROR: rational solve without Boost not defined!" << std::endl;)
   return 0;
#else
   size_t densize = msb(denominator(R)) + 1;
   size_t numsize = msb(numerator(R)) + 1;

   return (int)(densize + numsize);
#endif
}
/// Total size of rational vector.
inline int totalSizeRational(const Rational* vector, const int length, const int base)
{
   assert(vector != 0);
   assert(length >= 0);
   assert(base >= 0);

   int size = 0;

   for(int i = 0; i < length; i++)
      size += sizeInBase(vector[i], base);

   return size;
}

/// Size of least common multiple of denominators in rational vector.
inline int dlcmSizeRational(const Rational* vector, const int length, const int base)
{
   assert(vector != 0);
   assert(length >= 0);
   assert(base == 2);

#ifndef SOPLEX_WITH_BOOST
   MSG_ERROR(std::cerr << "ERROR: rational solve without Boost not defined!" << std::endl;)
   return 0;
#else

   Integer lcm;

   for(int i = 0; i < length; i++)
      SpxLcm(lcm, lcm, denominator(vector[i]));

   int size = msb(lcm) + 1;

   return size;
#endif
}

/// Size of largest denominator in rational vector.
inline int dmaxSizeRational(const Rational* vector, const int length, const int base)
{
   assert(vector != 0);
   assert(length >= 0);
   assert(base == 2);
#ifndef SOPLEX_WITH_BOOST
   MSG_ERROR(std::cerr << "ERROR: rational solve without Boost not defined!" << std::endl;)
   return 0;
#else

   size_t dmax = 0;

   for(int i = 0; i < length; i++)
   {
      size_t dsize = msb(denominator(vector[i])) + 1;

      if(dsize > dmax)
         dmax = dsize;
   }

   return (int)dmax;
#endif
}

} // namespace soplex
#endif
//}
