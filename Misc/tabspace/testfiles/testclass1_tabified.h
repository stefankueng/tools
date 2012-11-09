// some comments at the start

/* empty line */

// line with a tab and four spaces
		

/* class declaration commented out
class TestClass{
	int test;
	void	something();	
};
*/

std::string s("     test	");
const char c = '	';
std::string s2("\"       	\"\"\'");

// now the real class
class TestClass1
{
public:
	void			Method1(int test,bool bTest=false);
	long			Method2(  CString sTest,	  
							  CString sTest2);
protected:
	myvar			some_variable;
private:
	string			Method3(size_t val = 0);  
	
	void				Method_4( int variable1,
								  int variable2);

};
