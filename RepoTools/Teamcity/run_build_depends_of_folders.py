#!/usr/bin/env python
import sys
import os
import argparse
import json
import time
import team_city_api
import stash_api

# The script runs the teamcity-build or executes the command if the
# changes in the pull-request affect the specified folder list

def __print( str ):
    sys.stdout.write("{0}\n".format(str))
    sys.stdout.flush()

def __print_teamcity_message( text, type, errorDetails = '' ):
    __print('##teamcity[message text=\'{}\' errorDetails=\'{}\' status=\'{}\']'.format( text, errorDetails, type) )

def __print_teamcity_set_parameter( name, value ):
    __print('##teamcity[setParameter name=\'{}\' value=\'{}\']'.format( name, value ) )

def __parser_args():
    arg_parser = argparse.ArgumentParser()

    arg_parser.add_argument( '--stash_api_version', default = '1.0' )
    arg_parser.add_argument( '--stash_project', default = 'DF' )
    arg_parser.add_argument( '--stesh_repo_name', default = 'dava.framework' )

    arg_parser.add_argument( '--stash_url', required = True )
    arg_parser.add_argument( '--teamcity_url', required = True )

    arg_parser.add_argument( '--login', required = True )
    arg_parser.add_argument( '--password', required = True )
##
    arg_parser.add_argument( '--convert_to_merge_requests', default = 'false', choices=[ 'true', 'false' ] )
    arg_parser.add_argument( '--framework_brunch', required = True )
    arg_parser.add_argument( '--client_brunch' )
##
    arg_parser.add_argument( '--check_folders', required = True  )
##
    arg_parser.add_argument( '--configuration_id'  )
    arg_parser.add_argument( '--root_configuration_id'  )
    arg_parser.add_argument( '--request_stash_mode', default = 'false', choices=[ 'true', 'false' ] )
    arg_parser.add_argument( '--teamcity_freq_requests', default = 60, type = int  )
##
    arg_parser.add_argument( '--run_command'  )

    return arg_parser.parse_args()


def __run_build( args, triggering_options = [] ):
    teamcity = team_city_api.ptr()

    client_brunch    = args.client_brunch
    framework_brunch = args.framework_brunch

    if args.convert_to_merge_requests == 'true':
        if client_brunch and '/from' in client_brunch:
            client_brunch = client_brunch.replace('/from', '/merge')

        if framework_brunch and '/from' in framework_brunch:
            framework_brunch = framework_brunch.replace('/from', '/merge')

    properties = {}
    if client_brunch and client_brunch != '<default>':
        properties = {'client_branch': client_brunch}

    run_build_result = teamcity.run_build( args.configuration_id, framework_brunch, properties, triggering_options  )

    return run_build_result


def __wait_end_build( args, build_id ):#run_build_result['id']
    teamcity = team_city_api.ptr()

    build_status = ''
    build_status_text = ''

    teamcity_build_status = {}

    while  build_status != 'finished':

        teamcity_build_status =  teamcity.get_build_status( build_id  )

        build_status          = teamcity_build_status['state']

        build_status_text_old = build_status_text
        build_status_text     = teamcity_build_status['statusText']

        if build_status_text != build_status_text_old:
            __print( "{} ..".format( build_status_text ) )

        time.sleep( args.teamcity_freq_requests )

    if( teamcity_build_status[ 'status' ] != 'SUCCESS' ):
        __print_teamcity_message( 'Build failed !!!', 'ERROR', teamcity_build_status['webUrl'] )


def get_pull_requests_number( brunch ):
    brunch     = brunch.split('/')
    brunch_len = len( brunch )

    pull_requests_number = None

    if brunch_len == 1:
        if brunch[0].isdigit():
            pull_requests_number = brunch[0]
    else:
        pull_requests_number = brunch[ brunch_len - 2 ]

    return pull_requests_number


def __check_depends_of_folders( args ):
    __print( "Check depends" )

    stash = stash_api.ptr()

    pull_requests_number = get_pull_requests_number( args.framework_brunch )

    if pull_requests_number == None  :
        __print( "Build is required, because brunch == {}".format( args.framework_brunch ) )
        return True,None

    brunch_info = stash.get_pull_requests_info( pull_requests_number )

    merged_brunch = brunch_info['toRef']['id'].split('/').pop()

    if merged_brunch != 'development' :
        __print( "Build is required, because brunch_toRef == {}".format( pull_requests_number ) )
        return True, brunch_info

    #changes folders check
    brunch_changes =  stash.get_pull_requests_changes( pull_requests_number )[ 'values' ]

    depends_folders = args.check_folders.split(';')

    for path_dep_folder in depends_folders:
        for path_brunch_folder in brunch_changes:
            path                =  path_brunch_folder['path']['parent']
            path                = os.path.realpath( path )
            realpath_dep_folder = os.path.realpath(path_dep_folder)

            if realpath_dep_folder in path:
                __print( "Build is required because changes affect folders {}".format( depends_folders ) )
                return True, brunch_info

    if args.configuration_id != None :
        __print( "Build [{}] it is possible not to launch".format( args.configuration_id ) )

    if args.run_command != None :
        __print( "Command [{}] it is possible not to launch".format( args.run_command ) )

    if args.configuration_id == None and args.run_command != None :
        __print( "Build it is possible not to launch" )

    return False, brunch_info

def main():

    args = __parser_args()

    stash_api.init(     args.stash_url,
                        args.stash_api_version,
                        args.stash_project,
                        args.stesh_repo_name,
                        args.login,
                        args.password )

    team_city_api.init( args.teamcity_url,
                        args.login,
                        args.password )

    stash    = stash_api.ptr()

    teamcity = team_city_api.ptr()

    check_depends, brunch_info = __check_depends_of_folders( args )
    if check_depends == True:
        if args.run_command != None :
            os.system( args.run_command )

        if args.configuration_id != None :
            run_build_result = __run_build( args, ['queueAtTop'] )

            if args.request_stash_mode == 'true':
                stash.report_build_status('INPROGRESS',
                                          args.configuration_id,
                                          args.configuration_id,
                                          run_build_result['webUrl'],
                                          brunch_info['fromRef']['latestCommit'],
                                          description="runing")
            else:
                __wait_end_build( args, run_build_result['id'] )


        __print_teamcity_set_parameter( 'env.build_required', 'true' )

    else:
        if brunch_info != None and args.request_stash_mode == 'true':

            build_status = teamcity.get_build_status( args.root_configuration_id )

            root_build_url = build_status['webUrl']

            stash.report_build_status('SUCCESSFUL',
                                      args.configuration_id,
                                      args.configuration_id,
                                      root_build_url,
                                      brunch_info['fromRef']['latestCommit'],
                                      description="auto")

        __print_teamcity_set_parameter( 'env.build_required', 'false' )



if __name__ == '__main__':
    main()
