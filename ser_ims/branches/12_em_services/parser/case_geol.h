/* 
 * case_geol.h
 * Andreea Ancuta Onofrei ancuta_onofrei@yahoo.com
 */


#ifndef CASE_GEOL_H
#define CASE_GEOL_H

#define ocation_CASE                                    \
        switch(LOWER_DWORD(val)) {                  \
        case _ion1_:                                \
	        hdr->type = HDR_GEOLOCATION_T; \
	        hdr->name.len = 11;                 \
	        return (p + 4);                     \
                                                    \
        case _ion2_:                                \
                hdr->type = HDR_GEOLOCATION_T; \
                p += 4;                             \
	        goto dc_end;                        \
        }

#define ocat_CASE        \
        switch(LOWER_DWORD(val)) { \
        case _ocat_ :               \
	        p += 4;            \
                val = READ(p);     \
                ocation_CASE;         \
	        goto other;        \
        }

#define geol_CASE        \
        p += 4;          \
        val = READ(p);   \
        ocat_CASE;        \
        goto other;


#endif /* CASE_GEOL_H */
