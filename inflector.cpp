#include "inflector.h"

QList<ReplacementRule> Inflector::plurals;
QList<ReplacementRule> Inflector::singulars;
QList<QString> Inflector::uncountables;
Inflector::Initializer Inflector::initializer;

QString Inflector::pluralize(const QString &word)
{
    QString out = QString(word);
    if ( (out.length() == 0)
        || (!uncountables.contains(word.toLower())) ) {
        for ( ReplacementRule &r : plurals ) {
            if ( r.find(word) ) {
                out = r.replace(word);
                break;
            }
        }
    }
    return out;
}

QString Inflector::singularize(const QString &word)
{
    QString out = QString(word);
    if (( out.length() == 0)
        || (!uncountables.contains(out.toLower())) ) {
        for ( ReplacementRule &r : singulars ) {
            if ( r.find(word) ) {
                out = r.replace(word);
                break;
            }
        }
    }
    return out;
}

void Inflector::irregular(const QString &singular, const QString &plural)
{
    QString regexp, repl;
    
    if ( singular.mid(0, 1).toUpper() ==
        plural.mid(0, 1).toUpper() ) {
        // singular and plural start with the same letter
        regexp = "(?i)(" + singular.mid(0, 1) + ")"
            + singular.mid(1) + "$";
        repl = "\\1" + plural.mid(1);
        plurals.push_front(ReplacementRule(regexp, repl));

        regexp = "(?i)(" + plural.mid(0, 1) + ")"
            + plural.mid(1) + "$";
        repl = "\\1" + singular.mid(1);
        singulars.push_front(ReplacementRule(regexp, repl));
    } else {
        // singular and plural don't start with the same letter
        regexp = singular.mid(0, 1).toUpper() + "(?i)"
            + singular.mid(1) + "$";
        repl = plural.mid(0, 1).toUpper()
            + plural.mid(1);
        plurals.push_front(ReplacementRule(regexp, repl));

        regexp = singular.mid(0, 1).toLower() + "(?i)"
            + singular.mid(1) + "$";
        repl = plural.mid(0, 1).toLower()
            + plural.mid(1);
        plurals.push_front(ReplacementRule(regexp, repl));

        regexp = plural.mid(0, 1).toLower() + "(?i)"
            + plural.mid(1) + "$";
        repl = singular.mid(0, 1).toUpper()
            + singular.mid(1);
        singulars.push_front(ReplacementRule(regexp, repl));

        regexp = plural.mid(0, 1).toLower() + "(?i)"
            + plural.mid(1) + "$";
        repl = singular.mid(0, 1).toLower()
            + singular.mid(1);
        singulars.push_front(ReplacementRule( regexp, repl));
    }
}

Inflector::Initializer::Initializer()
{
    plurals.push_front(ReplacementRule("$", "s"));
    plurals.push_front(ReplacementRule("(?i)s$", "s"));
    plurals.push_front(ReplacementRule("(?i)(ax|test)is$", "\\1es"));
    plurals.push_front(ReplacementRule("(?i)(octop|vir)us$", "\\1i"));
    plurals.push_front(ReplacementRule("(?i)(alias|status)$", "\\1es"));
    plurals.push_front(ReplacementRule("(?i)(bu)s$", "$1es"));
    plurals.push_front(ReplacementRule("(?i)(buffal|tomat)o$", "\\1oes"));
    plurals.push_front(ReplacementRule("(?i)([ti])um$", "$1a"));
    plurals.push_front(ReplacementRule("sis$", "ses"));
    plurals.push_front(ReplacementRule("(?i)(?:([^f])fe|([lr])f)$", "\\1\\2ves"));
    plurals.push_front(ReplacementRule("(?i)(hive)$", "\\1s"));
    plurals.push_front(ReplacementRule("(?i)([^aeiouy]|qu)y$", "\\1ies"));
    plurals.push_front(ReplacementRule("(?i)(x|ch|ss|sh)$", "\\1es"));
    plurals.push_front(
                ReplacementRule("(?i)(matr|vert|ind)(?:ix|ex)$", "\\1ices"));
    plurals.push_front(ReplacementRule("(?i)([m|l])ouse$", "\\1ice"));
    plurals.push_front(ReplacementRule("^(?i)(ox)$", "\\1en"));
    plurals.push_front(ReplacementRule("(?i)(quiz)$", "\\1zes"));

    singulars.push_front(ReplacementRule("s$", ""));
    singulars.push_front(ReplacementRule("(n)ews$", "\\1ews"));
    singulars.push_front(ReplacementRule("([ti])a$", "\\1um"));
    singulars.push_front(ReplacementRule(
        "((a)naly|(b)a|(d)iagno|(p)arenthe|(p)rogno|(s)ynop|(t)he)ses$",
        "\\1\\2sis"));
    singulars.push_front(ReplacementRule("(^analy)ses$", "\\1sis"));
    singulars.push_front(ReplacementRule("([^f])ves$", "\\1fe"));
    singulars.push_front(ReplacementRule("(hive)s$", "\\1"));
    singulars.push_front(ReplacementRule("(tive)s$", "\\1"));
    singulars.push_front(ReplacementRule("([lr])ves$", "\\1f"));
    singulars.push_front(ReplacementRule("([^aeiouy]|qu)ies$", "\\1y"));
    singulars.push_front(ReplacementRule("(s)eries$", "\\1eries"));
    singulars.push_front(ReplacementRule("(m)ovies$", "\\1ovie"));
    singulars.push_front(ReplacementRule("(x|ch|ss|sh)es$", "\\1"));
    singulars.push_front(ReplacementRule("([m|l])ice$", "\\1ouse"));
    singulars.push_front(ReplacementRule("(bus)es$", "\\1"));
    singulars.push_front(ReplacementRule("(o)es$", "\\1"));
    singulars.push_front(ReplacementRule("(shoe)s$", "\\1"));
    singulars.push_front(ReplacementRule("(cris|ax|test)es$", "\\1is"));
    singulars.push_front(ReplacementRule("(octop|vir)i$", "\\1us"));
    singulars.push_front(ReplacementRule("(alias|status)es$", "\\1"));
    singulars.push_front(ReplacementRule("(ox)en$", "\\1"));
    singulars.push_front(ReplacementRule("(virt|ind)ices$", "\\1ex"));
    singulars.push_front(ReplacementRule("(matr)ices$", "\\1ix"));
    singulars.push_front(ReplacementRule("(quiz)zes$", "\\1"));

    irregular("person", "people");
    irregular("man", "men");
    irregular("child", "children");
    irregular("sex", "sexes");
    irregular("move", "moves");
    irregular("cow", "kine");

    uncountables.push_back("equipment");
    uncountables.push_back("information");
    uncountables.push_back("rice");
    uncountables.push_back("money");
    uncountables.push_back("species");
    uncountables.push_back("series");
    uncountables.push_back("fish");
    uncountables.push_back("sheep");
}
