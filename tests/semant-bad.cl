class C {
        d : Int <- a;
	a : Int;
        b : Int <- "";
        a : String <- 1;
        a : String <- c;
        c : T <- self.init(1);
	b : Bool;
        f(x : Int) : Int { x };
        g(x : String, i : Int) : Int { i };

        h1(x : Int, x : Int) : Int { 1 };
        h1(x : Int) : Int { 1 };
        h1(x : Int) : Int { 1 };
        h1(x : Int) : Int { "" };
        h2(x : Int) : Int { "" };

        h3(x : Int) : Int { 1 };
        h3(x : Int) : Int { 2 };

        self() : Int { 1 };

	init(x : Int, y : Bool) : C {
           {
                self.self();
                self.g("", 1);
                self.g("");
                self.g(2, 1);
                self.g(2, "");
                self.f();
                self.f(1);
                self.f(1,2);
                x@Int.y();
                x@Int.type_name();
                x@Int.type_name(11);
                self@TTT.init(1, false);
                self@SELF_TYPE.init(1, false);
                self@Int.init(1, false);
                self.init(1, false);
                case 0 of x : TTT => 0 ; esac ;
                case 1 of x : SELF_TYPE => 0; x : Int => 1; x : Int => 2; esac;
                case 2 of self : SELF_TYPE => 0; esac ;
                let a : Int, a : String in 1;
                let a : Int <- 1 in 1;
                let a : T in 1 + a;
                let self : XXXXXXXX in 1;
                if 1 then 1 else 2 fi;
                while 1 loop 2 pool;
                self <- self;
                t <- 0;
                a <- true;
                ~1;
                ~"";
                1=true;
                true=false;
                true + 1;
                not 1;
                not false;
                true < false;
                1 <= false;
                i;
                new T;
		a <- x + z;
		b <- y;
		self;
           }
	};
};

(* class A1 inherits Object {};
class A2 inherits IO {};
class A3 inherits Int {};
class A4 inherits String {};
class A5 inherits Bool {}; *)

-- class I inherits Int {};

-- class SELF_TYPE {};

-- class Z inherits Int {};

class Y {
  x : Int;
  -- x : String;
  y : SELF_TYPE;
  z : T;
};

class X inherits Y {
  -- x : Int;
};

-- class D inherits E {};

-- class E inherits D {};

Class Main {
	main():C {
	  (new C).init(1,true)
	};
};