#!/usr/bin/env ruby


# Copyright (c) 2007 Michael Haupt, Tobias Pape
# Software Architecture Group, Hasso Plattner Institute, Potsdam, Germany
# http://www.hpi.uni-potsdam.de/swa/
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
# $Id: som2lib.rb 169 2008-01-03 15:07:48Z tobias.pape $
 
def usage
  print <<EOUSAGE

#{$0} <Class.som>

generates lodable primitive from som-file for class Class.


EOUSAGE
end



class SomConverter
  def initialize( name )
    
    @cont = IO.readlines( name )
    
    @class = get_class()
    @includes = get_includes()
    @init = get_init()
    @fini = get_fini()
    @primitives = get_implementation()
    @decls = decl()
  end
  
  def to_h
    return { 
      "class_name" => @class,
      "includes" => @includes,
      "init_function" => @init,
      "fini_function" => @fini,
      "decls" => @decls,
      "primitives" => @primitives
    }
  end

  def to_s
    return "<class_name: " << @class << 
      ",\n includes: [" << @includes.join(",") <<
      "],\n init_function: " << @init.inspect <<
      "],\n fini_function: " << @fini.inspect <<
      ",\n decls: [" << @decls.join(",") <<
      ",\n primitives " << @primitives.inspect << ">"
  end

  private
  # get the class name out of 
  def get_class()
    result = nil
    @cont.each { |line|
      if line =~ /\s*\w+\s*=/
        result = /\w+/.match(line)[0]
        break
      end
    }
    return result
  end

  def get_includes()
    @cont.select { |line| 
      line =~ /@include/
    }. collect { |line| 
      (/(@include\s+)([\w\.<>\/\\']+)/.match(line)[2]).gsub("\\'",'"')
    }
  end

  def get_init()
    in_func = false
    result = Hash.new
    name=nil

    @cont.each { |line|
      if not in_func 
        if line =~ /@init\{/ 
           
          in_func=true
          name="__#{@class}_init"
          
          result[name]= ["void #{name}(void){\n"]
        end
      else
        if line =~ /\}@/
          result[name] << line.sub('}@','}').gsub("\\'",'"')
          in_func=false
          name=nil
        else
          result[name] << line.gsub("\\'",'"')
        end        
      end
    }
    return result
  end
  
  def get_fini()
    in_func = false
    result = Hash.new
    name=nil

    @cont.each { |line|
      if not in_func 
        if line =~ /@fini\{/ 
           
          in_func=true
          name="__#{@class}_fini"
          
          result[name]= ["void #{name}(void){\n"]
        end
      else
        if line =~ /\}@/
          result[name] << line.sub('}@','}').gsub("\\'",'"')
          in_func=false
          name=nil
        else
          result[name] << line.gsub("\\'",'"')
        end        
      end
    }
    
    return result
    
  end

  def get_implementation()
    in_func = false
    result = Hash.new
    name=nil
    class_side = false

    @cont.each { |line|
      
      if not in_func 
        if line =~ /\"@/ 
          in_func=true
          
          if line =~ /:/  # we got options
            name=  /(.*)\s+=\s+primitive/.match(line)[1].split.select { |field|
              field =~ /:/
            }.join("") # MAAAAAGIC
          else
            name=/([\w_]+)\s+=\s+primitive/.match(line)[1]
          end
          
          result[name] = ["void #{class_side ? '' : '_'}#{@class}_#{plain_string(name)}(pVMObject object, pVMFrame frame){\n"]
        elsif line =~ /\-\-\-\-/
          class_side = true
        end
      else
        if line =~ /@\"/
          result[name] << "}"
          in_func=false
          name=nil
        else
          result[name] << "\t" << line.gsub("\\'",'"')
        end        
      end
      
    }
 
    #p result.inspect
    return result
    
  end

  def decl

    result =  @init.to_a.collect { |name|
      "void #{name[0]}(void)"
    } 

    result << @fini.to_a.collect { |name|
      "void #{name[0]}(void)"
    }

    @primitives.each_key { | name |    
      result << "void _#{@class}_#{name.tr(':','_')}(pVMObject object, pVMFrame frame)"
    }

    return result
  end
  
  def plain_string(str)
    SomConverter.plain_string(str)
  end
  
public
  def SomConverter.plain_string(str)
    string = str.clone
    { "~" => "tilde",
      "&" => "and",
      "|" => "bar",
      "*" => "star",
      "/" => "slash",
      "@" => "at",
      "+" => "plus",
      "-" => "minus",
      "=" => "equal",     
      ">" => "greaterthan",
      "<" => "lowerthan",
      "," => "comma",
      "%" => "percent",
      "\\" => "backslash",
      ":" => "_"}.each do |k, v|
        string.gsub!(k,v)
      end
      return string
  end
  
  def SomConverter.generate_file_content(template)
###############################################################################
  template_content=<<EOT1
/*
 *  #{template["class_name"]}.c
 *  CSOM
 *
 *
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>


/*************************************************/
#pragma mark * Included Headers                  *
/*************************************************/

EOT1

template["includes"].each {|incl| 
    template_content << '#include ' << incl << "\n"  
}

template_content << <<EOT

/*************************************************/
#pragma mark * Primitive Foreward Declaration    *
/*************************************************/

#{template["decls"].each do |dec| dec<<";\n" end }

/*************************************************/
#pragma mark * Internal functions and init.      *
/*************************************************/

/*** Lib initialization **/
#ifdef __GNUC__
void init(void) __attribute__((constructor));
void fini(void) __attribute__((destructor));
#elif
void _init(void);
void _fini(void);
#pragma init _init
#pragma fini _fini
#endif

#ifdef __GNUC__
void init(void)
#elif
void _init(void)
#endif
{
	// Call init funcions.
	#{template["init_function"].to_a.collect {|func| func[0] + "();\n" }.to_s}

}

#ifdef __GNUC__
void fini(void)
#elif
void _fini(void)
#endif
{
	#{template["fini_function"].to_a.collect {|func| func[0] + "();\n" }.to_s}
}

// Classes supported by this lib.
static char *supported_classes[] = {
    "#{template["class_name"]}",
    NULL
};


/*************************************************/
#pragma mark * Exported functions starting here  *
/*************************************************/

// returns, whether this lib is responsible for a specific class
bool		supports_class(const char* name) {
	
	char **iter=supported_classes;
	while(*iter)
		if (strcmp(name,*iter++)==0)
			return true;
	return false;
	
}



/*************************************************/
/*************************************************/
/*************************************************/
#pragma mark * Primitive Implementatition here   *
/*************************************************/
/*************************************************/
/*************************************************/

/******* initialize ******************************/

#{template["init_function"].to_a.collect {|func| func[1].join("")}.to_s}

#{template["fini_function"].to_a.collect {|func| func[1].join("")}.to_s}

#{template["primitives"].to_a.collect { |p| p[1].join("") }.join("\n\n\n")}

/*************************************************/
/*************************************************/
/*************************************************/
#pragma mark * EOF                               *
/*************************************************/
/*************************************************/
/*************************************************/

EOT

    return template_content
###############################################################################
  end

  def generate_file_content!
    @cont = SomConverter.generate_file_content(self.to_h)
  end

  def write_out
    File.open(@class+".c","w") { |file|
      file.write(@cont)
    }
    return @class+".c"
  end

end

def main 
  if ARGV.size != 1
    usage
    exit
  end
 
  sc = SomConverter.new(ARGV[0])
  sc.generate_file_content!
  sc.write_out
  
end

if __FILE__ == $0
  main
end
